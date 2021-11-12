#pragma once

#include "core/os.hpp"
#include "core/filesystem.hpp"

#ifdef _WINDOWS
    #pragma warning(disable:4005)
    #pragma comment(linker, "/nodefaultlib:libcmt")
    
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "shlwapi.lib")
    #pragma comment(lib, "winmm.lib")
#endif

/*** main function macro ***/
#ifdef _WINDOWS
    #define __main() \
    int luz_main(std::vector<std::string> &args);\
    int wmain(int argc, wchar_t **argv) {\
        initlocale();\
        std::vector<std::string> args = os_cmdline(argv, argc);\
        return luz_main(args);\
    }\
    int luz_main(std::vector<std::string> &args)
#else
    #define __main() \
    int luz_main(std::vector<std::string> &args);\
    int main(int argc, char **argv) {\
        initlocale();\
        std::vector<std::string> args = os_cmdline(argv, argc);\
        return luz_main(args);\
    }\
    int luz_main(std::vector<std::string> &args)
#endif

/*** utility macro ***/
#ifdef _WINDOWS
    #define _U8(str) wcstou8(L##str)
    #define _S(str) L##str
#else
    #define _U8(str) std::string(str)
    #define _S(str) str
#endif

#define _fputs(fp, str) fputws((u8towcs(str) + L"\n").c_str(), fp)
#define _fprintf(fp, format, ...) fwprintf(fp, u8towcs(format).c_str(), __VA_ARGS__)

/*** include source files macro ***/
#ifdef _USE_LUZ_CORE
    #include "core/string.cpp"
    #include "core/path.cpp"
    #include "core/os.cpp"
    #include "core/filesystem.cpp"
#endif
