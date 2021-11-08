#pragma once

#include "luz/os.hpp"
#include "luz/filesystem.hpp"

#ifdef _WINDOWS
    #pragma warning(disable:4005)
    
    #pragma comment(lib, "user32.lib")
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "shlwapi.lib")
    #pragma comment(lib, "winmm.lib")

    #pragma comment(lib, "libluz.lib")
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
