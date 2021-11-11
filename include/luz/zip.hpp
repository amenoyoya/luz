#pragma once

#include "zip/archiver.hpp"

#ifdef _WINDOWS
    #pragma comment(lib, "zlib.lib")
#endif

/*** include source files macro ***/
#ifdef _USE_LUZ_ZIP
    #include "zip/archiver.cpp"
#endif
