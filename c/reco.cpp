#define _USE_LUZ_CORE
#define _USE_LUZ_ZIP
#define _USE_LUZ_LUA
#include <luz/lua.hpp>
#include "recolib.inc"
#include "recolib_stable.inc"

/// general smart pointer
typedef std::unique_ptr<void, std::function<void(void*)>> record_ptr;

/// record: memory management system
struct record {
    record_ptr  handler;
    size_t      size;
    
    record(const std::function<std::tuple<unsigned long, size_t>(void)> &allocator, const std::function<void(void*)> &deleter)
        : handler(nullptr), size(0)
    {
        auto alloc = allocator();
        this->handler = record_ptr((void*)std::get<0>(alloc), deleter);
        this->size = std::get<1>(alloc);
    }
    ~record() {
        this->close();
    }

    void close() {
        this->handler.reset();
        this->size = 0;
    }

    unsigned long addr() { return (unsigned long)this->handler.get(); }
    std::string tostr() { return this->handler ? (const char*)this->handler.get() : ""; }
};

/// normal record factory
inline std::unique_ptr<record> reco_new(size_t size, const void *data = nullptr) {
    return std::unique_ptr<record>(new record(
        [&size, &data]() {
            auto ptr = new char[size + 1](); // allocate memory (+1 buffer for the end null pointer) and set to 0
            if (data) memcpy(ptr, data, size);
            return std::make_tuple((unsigned long)ptr, size);
        },
        [](void *ptr) {
            char *data = (char*)ptr;
            delete [] data;
        }
    ));
}

__main() {
    sol::state lua;
    
    /// open core libraries
    lua.open_libraries(
        sol::lib::base,
        sol::lib::coroutine,
        sol::lib::package,
        sol::lib::string,
        sol::lib::table,
        sol::lib::io,
        sol::lib::math,
        sol::lib::os,
        sol::lib::debug,
        sol::lib::ffi,
        sol::lib::bit32,
        sol::lib::jit
    );

    /// record: memory management system
    auto reco = lua["reco"].get_or_create<sol::table>();
    reco.new_usertype<record>("record",
        sol::constructors<record(const std::function<std::tuple<unsigned long, size_t>(void)>&, const std::function<void(void*)>&)>(),
        "size", &record::size,
        "close", &record::close,
        "addr", &record::addr,
        "tostr", &record::tostr,
        "cast", [&lua](record *self, const std::string &ctype) {
            return lua.safe_script("return ffi.cast('" + ctype + "', " + tostr(self->addr()) + ")");
        }
    );
    reco.set_function("new", sol::overload(
        [](size_t size, const void *data) { return reco_new(size, data); },
        [](size_t size) { return reco_new(size); }
    ));

    /// os.argv <= args (std::vector<std::string(UTF-8)>)
    auto os = lua["os"].get_or_create<sol::table>();
    os["argv"] = sol::as_table(args);

    /// lua standard libraries
    if(!openscript(lua, corelib_stable, "@stdlib://core")) return 1;

    /// main script: os.argv[1].lua
    // * package.__file <= os.argv[1]
    // * package.__dir <= parent directory of os.argv[1]
    if (args.size() < 2) {
        lua_dotty(lua);
        return 0;
    }
    auto package = lua["package"].get_or_create<sol::table>();
    package["__file"] = path_complete(args[1]);
    package["__dir"] = path_parentdir(package["__file"]);
    auto result = lua.safe_script_file(args[1]);
    if (result.valid()) return 0;

    sol::error err = result;
    _fputs(stderr, err.what());
    return 1;
}