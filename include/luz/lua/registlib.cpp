#include "../lua.hpp"

#include "luautf8/lutf8lib.c"
#include "lpeg-1.0.0/lpcap.c"
#include "lpeg-1.0.0/lpcode.c"
#include "lpeg-1.0.0/lpprint.c"
#include "lpeg-1.0.0/lptree.c"
#include "lpeg-1.0.0/lpvm.c"

/* Lua extended standard libraries */
static unsigned char core_lib_code[] = {
    #include "stdlib/core.cpp"
}, table_lib_code[] = {
    #include "stdlib/table.cpp"
}, string_lib_code[] = {
    #include "stdlib/string.cpp"
}, os_lib_code[] = {
    #include "stdlib/os.cpp"
}, filesystem_lib_code[] = {
    #include "stdlib/filesystem.cpp"
}, zip_lib_code[] = {
    #include "stdlib/zip.cpp"
}, lpeg_lib_code[] = {
    #include "stdlib/lpeg.cpp"
};

/// @private execute lua byte-code
static bool exec_lua_buffer(sol::state &lua, const char *buffer, size_t bufferSize, const std::string &scriptName, std::string *errorMessage) {
    auto script = lua.load_buffer(buffer, bufferSize, scriptName.c_str());
    if (!script.valid()) {
        sol::error err = script;
        *errorMessage = strtou8(err.what());
        return false;
    }

    auto result = script();
    if (!result.valid()) {
        sol::error err = result;
        *errorMessage = strtou8(err.what());
        return false;
    }
    return true;
}

/// @private register lua extended standard libraries
static bool regist_lua_stdlib(sol::state &lua, std::string *errorMessage) {
    /// oberload `debug.debug()`: call lua_dotty
    auto debug = lua["debug"].get_or_create<sol::table>();
    debug["debug"].set_function(sol::overload(
        [&lua](const std::string &progname) { lua_dotty(lua, progname); },
        [&lua]() { lua_dotty(lua); }
    ));

    if (!exec_lua_buffer(lua, (const char *)core_lib_code, sizeof(core_lib_code), "@stdlib://core", errorMessage)) return false;
    if (!exec_lua_buffer(lua, (const char *)table_lib_code, sizeof(table_lib_code), "@stdlib://table", errorMessage)) return false;
    if (!exec_lua_buffer(lua, (const char *)string_lib_code, sizeof(string_lib_code), "@stdlib://string", errorMessage)) return false;
    if (!exec_lua_buffer(lua, (const char *)os_lib_code, sizeof(os_lib_code), "@stdlib://os", errorMessage)) return false;
    if (!exec_lua_buffer(lua, (const char *)filesystem_lib_code, sizeof(filesystem_lib_code), "@stdlib://filesystem", errorMessage)) return false;
    if (!exec_lua_buffer(lua, (const char *)zip_lib_code, sizeof(zip_lib_code), "@stdlib://zip", errorMessage)) return false;
    if (!exec_lua_buffer(lua, (const char *)lpeg_lib_code, sizeof(lpeg_lib_code), "@stdlib://lpeg", errorMessage)) return false;
    return true;
}

bool lua_registlib(sol::state &lua, std::string *errorMessage) {
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
    // UTF-8 support
    luaopen_utf8(lua);
    lua_setglobal(lua, "utf8"); // set to global `utf8` table
    // LPEG support
    luaopen_lpeg(lua); // to use `require"ffi"`
    // register lua extended standard libraries
    return regist_lua_stdlib(lua, errorMessage);
}