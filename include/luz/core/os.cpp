#include "os.hpp"

#ifdef _WINDOWS
    #include <mmsystem.h>
#else
    #include <sys/time.h>
#endif

extern "C" {
    __export long os_execute(const char *cmd) {
        #ifdef _WINDOWS
            return _wsystem(u8towcs(cmd).c_str());
        #else
            return /*WEXITSTATUS(*/ system(cmd) /*)*/;
        #endif
    }

    __export void os_sleep(unsigned long msec) {
        #ifdef _WINDOWS
            Sleep(msec);
        #else
            usleep(msec * 1000);
        #endif
    }

    __export unsigned long os_gettime() {
        #ifdef _WINDOWS
            return timeGetTime();
        #else
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            return tv.tv_sec * 1000 + tv.tv_usec / 1000;
        #endif
    }

    __export bool os_setenv(const char *name, const char *val) {
        #ifdef _WINDOWS
            return FALSE != SetEnvironmentVariableW(u8towcs(name).c_str(), u8towcs(val).c_str());
        #else
            return 0 == putenv((char*)(std::string(name) + "=" + val).c_str());
        #endif
    }

    __export const char *os_getenv(const char *env) {
        #ifdef _WINDOWS
            static std::string result;

            std::wstring name = std::move(u8towcs(env)), buf;
            unsigned long size = GetEnvironmentVariable(name.c_str(), nullptr, 0);
            
            if(size == 0) return nullptr;
            buf.resize(size);
            GetEnvironmentVariable(name.c_str(), (wchar_t*)buf.c_str(), size);
            result = std::move(wcstou8(buf));
            return result.c_str();
        #else
            return getenv(env);
        #endif
    }

    __export bool os_setcwd(const char *dir) {
        #ifdef _WINDOWS
            return FALSE != SetCurrentDirectoryW(u8towcs(dir).c_str());
        #else
            return 0 == chdir(dir);
        #endif
    }

    __export const char *os_getcwd(char *dest, size_t size) {
        #ifdef _WINDOWS
            wchar_t *buffer = new wchar_t[size];
            
            GetCurrentDirectory(size, buffer);
            wcstou8(dest, buffer, size);
            delete [] buffer;
            return dest;
        #else
            return getcwd(dest, size);
        #endif
    }
}
