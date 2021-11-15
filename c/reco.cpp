#define _USE_LUZ_CORE
#define _USE_LUZ_ZIP
#define _USE_LUZ_LUA
#include <luz/lua.hpp>

/// record: memory management system
struct record_t {
    unsigned long   handler;
    size_t          size;
};
typedef std::unique_ptr<record_t,std::function<void(record_t*)>> record_ptr;

inline void reco_deleter(record_t *self, const std::function<void(unsigned long, size_t)> &custom_deleter) {
    if (!self) return;
    if (self->handler) custom_deleter(self->handler, self->size);
    delete self;
    self = nullptr;
}

inline void reco_delete(record_t *self) {
    reco_deleter(self, [](unsigned long handler, size_t size) {
        char *ptr = (char*)handler;
        delete [] ptr;
        ptr = nullptr;
    });
}

inline record_ptr reco_new(size_t size, const void *data = nullptr) {
    auto ptr = new char[size + 1](); // allocate memory and set to 0
    auto self = record_ptr(new record_t { (unsigned long)ptr, size }, reco_delete);
    if (!self) {
        delete [] ptr;
        return nullptr;
    }
    if (data) memcpy(ptr, data, size);
    return std::move(self);
};

inline void reco_close(record_ptr &self) {
    self.reset();
}

inline std::string reco_tostr(record_t *self) {
    return self && self->handler ? (const char*)self->handler : "";
}

/// lua core library
const char *corelib = R"(
--- Serialize ---
local escape_char_map = {
    [ "\\" ] = "\\",
    [ "\"" ] = "\"",
    [ "\b" ] = "b",
    [ "\f" ] = "f",
    [ "\n" ] = "n",
    [ "\r" ] = "r",
    [ "\t" ] = "t",
}

local function escape_char(c)
    return "\\" .. (escape_char_map[c] or string.format("u%04x", c:byte()))
end

-- @private Process of escaping
local function escape_string(str)
    return str:gsub('[%z\1-\31\\"]', escape_char)
end

-- @private Stringify Lua value
-- @param {boolean} all: all values will be stringified if `true`
local function value2str(v, all)
    local vt = type(v)
    
    if vt == 'nil' then
        return "nil"
    elseif vt == 'number' then
        return tostring(v)
    elseif vt == 'string' then
        return '"' .. escape_string(v) .. '"'
    elseif vt == 'boolean' then
        return tostring(v)
    elseif vt == 'table' then
        return tostring(v)
    elseif all then
        return tostring(v)
    end
    return ""
end

-- @private Stringify table's field
-- if field is number: return ''
-- if field is not number: return '["$field"]='
local function field2str(v)
    if type(v) == 'number' then return "" end
    return '["' .. escape_string(v) .. '"]='
end

-- @private Basic serialization
-- @param {table} tbl
-- @param {boolean} all: all values will be stringified recursively if `true`
-- @param {number} space_count: indent space count
-- @param {string} space: current indent space count
local function serialize_table(tbl, all, space_count, space)
    local buf = ""
    local f, v = next(tbl, nil)
    local next_space = space

    if space_count > 0 then
        next_space = space .. (" "):rep(space_count)
    end
    
    while f do
        if type(v) == 'table' then
            if buf ~= "" then
                buf = buf .. "," .. next_space
            end
            buf = buf.. field2str(f) .. (all and serialize_table(v, all, space_count, next_space) or value2str(v))
        else
            local value = value2str(v, all)
            if value then
                if buf ~= "" then
                    buf = buf .. "," .. next_space
                end
                buf = buf .. field2str(f) .. value
            end
        end
        -- next
        f, v = next(tbl, f)
    end

    return '{' .. next_space .. buf .. space .. '}'
end

-- Serialize the table
-- @param {number} space_count (default: 0): indent space count
-- @param {boolean} all (default: false): all values will be stringified if `true`
function table:serialize(space_count, all)
    if type(self) ~= 'table' then
        return value2str(self, all) -- not table
    end

    space_count = space_count == nil and 0 or space_count
    all = all or false
    return serialize_table(self, all, space_count, space_count > 0 and "\n" or "")
end

--- FFI functions ---
ffi.cdef[[
bool u8towcs(wchar_t *dest, const char *source, size_t size);
bool wcstou8(char *dest, const wchar_t *source, size_t size);
size_t wcslen(const wchar_t *str);
int fputws(const wchar_t *str, struct FILE *fp);
wchar_t *fgetws(wchar_t *dest, size_t n, struct FILE *fp);
]]

-- UTF-8 string to wide string (UTF-16)
function string:u8towcs()
    local size = self:len()
    local dest = ffi.new("wchar_t[?]", size + 1) -- +1 buffer for the end of null pointer
    ffi.C.u8towcs(dest, self, size)
    return dest
end

-- Wide string (UTF-16) to UTF-8 string
function string.wcstou8(src)
    local size = ffi.C.wcslen(src) * 3 -- 1 char of utf8: max 3 byte
    local dest = ffi.new("char[?]", size + 1) -- +1 buffer for the end of null pointer
    ffi.C.wcstou8(dest, src, size)
    return ffi.string(dest)
end

-- @private General print function
-- * support UTF-8 for Windows
-- * support to print nil
local function fprint(stdout, ...)
    local list = {...} -- #list returns 1 if the list is like {1, nil, 3}
    local key = 0
    -- ipairs can't scan all elements if the list is like {1, nil, 3} (scan only 1)
    for k, v in pairs(list) do
        if key > 0 then ffi.C.fputws(string.u8towcs("\t"), stdout) end
        if k > key + 1 then
            for i = 1, k - key - 1 do
                ffi.C.fputws(string.u8towcs("nil\t"), stdout)
            end
        end
        ffi.C.fputws(
            string.u8towcs(
                type(list[k]) == 'table' and table.serialize(list[k], 2, true) or tostring(list[k])
            ),
            stdout
        )
        key = k
    end
    ffi.C.fputws(string.u8towcs((key == 0 and "nil" or "") .. "\n"), stdout)
end

-- Override default `print` function
function print(...)
    fprint(io.stdout, ...)
end

-- Print formatted string
function printf(format, ...)
    ffi.C.fputws(format:format(...):u8towcs(), io.stdout)
end

-- Error print function
function eprint(...)
    fprint(io.stderr, ...)
end

function eprintf(format, ...)
    ffi.C.fputws(format:format():u8towcs(), io.stderr)
end

-- Print formatted string to stderr
function errorf(format, ...)
    return error(format:format(...))
end

-- Read string from stdin (support UTF-8 for Windows)
function readln(message, size)
    if type(message) == 'string' then
        ffi.C.fputws(message:u8towcs(), io.stdout)
    end

    size = size or 1024
    local buf = ffi.new("wchar_t[?]", size + 1)
    return ffi.string(string.wcstou8(ffi.C.fgetws(buf, size, io.stdin))):remove() -- remove the end of line
end
)";

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
    auto corelib_result = lua.safe_script(corelib, "@stdlib://core");
    if (!corelib_result.valid()) {
        sol::error err = corelib_result;
        _fputs(stderr, err.what());
        return 1;
    }

    /// record: memory management system
    lua.new_usertype<record_t>("reco",
        "handler", &record_t::handler,
        "size", &record_t::size,
        "new", sol::overload(
            [](size_t size, const void *data) { return reco_new(size, data); },
            [](size_t size) { return reco_new(size); }
        ),
        "close", reco_close,
        "tostr", reco_tostr,
        "cast", [&lua](record_t *self, const std::string &ctype) {
            return lua.safe_script("return ffi.cast('" + ctype + "', " + tostr(self->handler) + ")");
        }
    );

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