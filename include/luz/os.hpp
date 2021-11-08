#pragma once

#include "path.hpp"
#include <vector>

extern "C" {
    /// execute os command
    __export long os_execute(const char *cmd);

    /// sleep [milli seconds]
    __export void os_sleep(unsigned long msec);

    /// get current os time [milli seconds]
    __export unsigned long os_gettime();

    /// set system environmental value
    __export bool os_setenv(const char *name, const char *val);

    /// get system environmental value
    __export const char *os_getenv(const char *name);

    /// set current working directory
    __export bool os_setcwd(const char *dir);
    
    /// get current working directory
    __export const char *os_getcwd(char *dest, size_t size);
}

/*** ================================================== ***/
/*** utility functions for C++ ***/

/// convert command line arguments to std::vector<std::string(UTF-8)>
template<typename Char>
std::vector<std::string> os_cmdline(Char *argv[], int argc) {
    std::vector<std::string> arguments;

    for (int i = 0; i < argc; ++i) {
        if (i == 0) {
            // Current execution file: => Get full path
            arguments.push_back(
                path_complete(
                    #ifdef _WINDOWS
                        wcstou8(argv[i])
                    #else
                        argv[i]
                    #endif
                )
            );
            continue;
        }
        arguments.push_back(
            #ifdef _WINDOWS
                wcstou8(argv[i])
            #else
                argv[i]
            #endif
        );
    }
    return std::move(arguments);
}