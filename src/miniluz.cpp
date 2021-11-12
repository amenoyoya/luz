#define _USE_LUZ_CORE
#define _USE_LUZ_ZIP
#define _USE_LUZ_LUA
#include <luz/zip.hpp>
#include <luz/lua.hpp>

__main() {
    sol::state lua;
    std::string errorMessage;

    /// register standard libraries
    if (!lua_registlib(lua, &errorMessage)) {
        _fputs(stderr, errorMessage);
        return 1;
    }

    /// os.argv <= args (std::vector<std::string(UTF-8)>)
    auto os = lua["os"].get_or_create<sol::table>();
    os["argv"] = sol::as_table(args);

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