#pragma once

#define UNICODE
#define SOL_ALL_SAFETIES_ON 1
#define SOL_LUAJIT 1
#include <sol/sol.hpp>
#include "core.hpp"

#ifdef _WINDOWS
    #pragma comment(lib, "lua51.lib")
#endif


/// Register Libraries
//  - Extended Lua standard libraries
//  - Luz core library
bool lua_registlib(sol::state &lua, std::string *errorMessage);

/// Execute interactive lua
void lua_dotty(sol::state &lua, const std::string &progname = "luz");


/*** include source files macro ***/
#ifdef _USE_LUZ_LUA
    #include <luz/lua/registlib.cpp>
    #include <luz/lua/dotty.cpp>
#endif
