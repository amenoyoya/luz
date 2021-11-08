#pragma once

#include "config.hpp"
#include <string>
#include <cstring>
#include <clocale>

extern "C" {
    #ifdef _WINDOWS
        /// set wide string mode to stdio (windows only)
        __export void io_setu16mode(FILE *fp); 
    #endif

    /// convert UTF-8 string to wide string (UTF-16)
    __export bool u8towcs(wchar_t *dest, const char *source, size_t size);
    
    /// convert wide string (UTF-16) to UTF-8 string
    __export bool wcstou8(char *dest, const wchar_t *source, size_t size);
}

/*** ================================================== ***/
/*** utility functions for C++ ***/

/// initialize locale
// on windows: set wide string mode to stdio file pointer (measures against garbled characters in multibyte languages)
inline void initlocale() {
    setlocale(LC_ALL, "");
    #ifdef _WINDOWS
        io_setu16mode(stdout);
        io_setu16mode(stderr);
        io_setu16mode(stdin);
    #endif
}

/// convert UTF-8 string to wide string (UTF-16)
inline std::wstring u8towcs(const std::string &source) {
    std::wstring dest;

    dest.resize(source.size() + 1); // +1 buffer for the last null char
    u8towcs((wchar_t*)dest.c_str(), source.c_str(), source.size());
    return dest.c_str();
}

/// convert wide string (UTF-16) to UTF-8 string
inline std::string wcstou8(const std::wstring &source) {
    std::string dest;

    // UTF-8 character will consume ~ 3 bytes
    dest.resize(source.size() * 3 + 1); // +1 buffer for the last null char
    wcstou8((char*)dest.c_str(), source.c_str(), source.size() * 3);
    return dest.c_str();
}
