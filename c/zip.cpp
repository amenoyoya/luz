#define _USE_LUZ_CORE
#define _USE_LUZ_ZIP
#include <luz/zip.hpp>

inline std::string getinput(const char *message) {
    std::wstring input;
    std::string result;

    _fprintf(stdout, message);
    input.resize(1025);
    fgetws((wchar_t*)input.c_str(), 1024, stdin);
    result = std::move(wcstou8(input));
    result.erase(result.end() - 1); // remove end of line
    return std::move(result);
}

__main() {
    if (!os_setcwd(path_parentdir(args[0]).c_str())) {
        _fputs(stderr, "failed to change current working directory");
        return 1;
    }
    if (!zip_compress(_U8("✅utf8dir").c_str(), "archive.zip", 9, "password", "w", "")) {
        _fputs(stderr, "failed to compress directory");
        return 1;
    }
    if (!unz_uncompress("archive.zip", _U8("❌archive").c_str(), "password")) {
        _fputs(stderr, "failed to uncompress zip");
        return 1;
    }
    
    std::string input = getinput("Are you want to delete test files ? (y/n) ");
    if (input == "y") {
        fs_rmdir(_U8("❌archive").c_str());
        fs_rmfile("archive.zip");
    }
    return 0;
}
