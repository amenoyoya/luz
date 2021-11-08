#include <luz.hpp>

#ifdef _WINDOWS
    #define _U8(str) wcstou8(L##str)
    #define _S(str) L##str
    #define _fputs(fp, str) fputws((u8towcs(str) + L"\n").c_str(), fp) 
#else
    #define _U8(str) std::string(str)
    #define _S(str) str
    #define _fputs(fp, str) fputs((_U8(str) + "\n").c_str(), fp)
#endif

inline std::string readfile(const std::string &filename) {
    FILE *fp = fs_fopen(filename.c_str(), "rb");
    if (!fp) return "";

    std::string str;
    size_t size;

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    str.resize(size + 1);
    fread((void*)str.c_str(), 1, size, fp);
    fclose(fp);
    return str.c_str();
}

__main() {
    if (!os_setcwd(path_parentdir(args[0]).c_str())) {
        _fputs(stderr, "failed to change current working directory");
        return 1;
    }
    
    char cwd[1024];
    _fputs(stdout, os_getcwd(cwd, 1024));
    
    FILE *fp = fs_fopen("test/subdir/test.txt", "wb");
    if (!fp) {
        _fputs(stderr, "failed to create file: test/subdir/test.txt");
        return 1;
    }
    std::string data = _U8("✨⚡");
    fwrite(data.c_str(), 1, data.size(), fp);
    fclose(fp);

    _fputs(stdout, "test/subdir/test.txt: " + readfile("test/subdir/test.txt"));

    if (!fs_copydir(_U8("✅utf8dir").c_str(), _U8("⚡copied").c_str())) {
        _fputs(stderr, "failed to copy directory");
        return 1;
    }

    _fputs(stdout, _U8("⚡copied/⭕utf8file.txt: ") + readfile(_U8("⚡copied/⭕utf8file.txt")));
    _fputs(stdout, _U8("⚡copied/❌u8subdir/subdir.file.txt: ") + readfile(_U8("⚡copied/❌u8subdir/subdir.file.txt")));

    if (!fs_rmdir(_U8("⚡copied").c_str())) {
        _fputs(stderr, _U8("failed to remove directory: ⚡copied"));
        return 1;
    }
    if (!fs_rmdir("test")) {
        _fputs(stderr, "failed to remove directory: test");
        return 1;
    }
    return 0;
}