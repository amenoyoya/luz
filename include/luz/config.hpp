#pragma once

// Windows OS
#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32_)
    #define _WINDOWS
    #pragma warning(disable: 4005)
#endif

// Linux OS
#ifdef _linux_
    #define _LINUX
#endif

// Mac OS
#ifdef _APPLE_
    #define _APPLE
#endif

// Android OS
#ifdef ANDROID_NDK
    #define _ANDROID
#endif


// Desktop Application
#if defined(_WINDOWS) || defined(_LINUX) || defined(_APPLE)
    #define _DESKTOP_APP
#endif

// Smartphone Application
#ifdef _ANDROID
    #define _SMARTPHONE_APP
#endif

// OS unique API
#ifdef _WINDOWS
    #define UNICODE
    #define STRICT
    #define _CRT_SECURE_NO_WARNINGS
    #include <windows.h>
#else
    #include <unistd.h>
#endif

// dllexport
#ifndef __export
    #ifdef _WINDOWS
        #define __export __declspec(dllexport)
    #else
        #define __export __attribute__((visibility ("default")))
    #endif
#endif
