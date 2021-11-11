#include "archiver.hpp"
#include <zlib.h>
#include <zip.h>
#include <unzip.h>
#include <time.h>

/// zip global information structure
typedef struct {
    size_t  entries,      // entry count
            comment_size; // global comment length
    std::string comment;  // global comment
} zip_global_info_t;

/// @private get file size
static size_t __get_filesize(FILE *fp) {
    size_t cur = ftell(fp), size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, cur, SEEK_SET); // restore file pointer position
    return size;
}

static size_t __get_filesize(const char *filename) {
    FILE *fp = fs_fopen(filename, "rb");
    if (fp == nullptr) return 0;
    size_t size = __get_filesize(fp);
    fclose(fp);
    return size;
}

/// @private read file content
static std::string __get_filecontent(const char *filename) {
    FILE *fp = fs_fopen(filename, "rb");
    if (fp == nullptr) return "";
    fseek(fp, 0, SEEK_END);

    std::string data;
    size_t size = ftell(fp);
    data.resize(size);
    fseek(fp, 0, SEEK_SET);
    fread((void *)data.c_str(), 1, size, fp);
    fclose(fp);
    return std::move(data);
}

/// @private get current date time
static bool __get_datetime(tm_zip *dest) {
    struct tm *t;
    time_t timer = (time_t)time(nullptr);
    
    if(nullptr == (t = localtime(&timer))) return false;
    dest->tm_year = t->tm_year + 1900;
    dest->tm_mon = t->tm_mon + 1;
    dest->tm_mday = t->tm_mday;
    dest->tm_hour = t->tm_hour;
    dest->tm_min = t->tm_min;
    dest->tm_sec = t->tm_sec;
    return true;
}

/// @private get DOS date
inline unsigned short __get_dosdate(unsigned long year, unsigned long month, unsigned long day) {
    return (unsigned short)(((unsigned)(year - 1980) << 9) | ((unsigned)month << 5) | (unsigned)day);
}

/// @private get DOS time
inline unsigned short __get_dostime(unsigned long hour, unsigned long minute, unsigned long second) {
    return (unsigned short)(((unsigned)hour << 11) | ((unsigned)minute << 5) | ((unsigned)second >> 1));
}

/// @private replace "\" to "/"
inline std::string &__exchange_path(std::string &dest) {
    for (char *p = (char*)dest.c_str(); *p; ++p) if(*p == '\\') *p = '/';
    return dest;
}

/*** @private zlib helper functions ***/
#ifdef _WINDOWS
    #define ENCODE(str) u8towcs(str).c_str()
    #define _FOPEN(open) _w##open
    #define _STRING(str) L##str
#else
    #define ENCODE(str) str
    #define _FOPEN(open) open
    #define _STRING(str) str
#endif
#define FOPEN(open, path, mode) _FOPEN(open)(ENCODE(path), mode)

static voidpf ZCALLBACK __open_file_func(voidpf opaque, const char* filename, int mode) {
    FILE *file = nullptr;
    #ifdef _WINDOWS
        const wchar_t* mode_fopen = nullptr;
    #else
        const char* mode_fopen = nullptr;
    #endif
    
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ) mode_fopen = _STRING("rb");
    else if (mode & ZLIB_FILEFUNC_MODE_EXISTING) mode_fopen = _STRING("r+b");
    else if (mode & ZLIB_FILEFUNC_MODE_CREATE) mode_fopen = _STRING("wb");
    
    if (mode_fopen) {
        if (mode_fopen[0] == 'w') fs_mkdir(path_parentdir(filename).c_str()); // create parent directories recursively
        file = FOPEN(fopen, filename, mode_fopen);
    }
    return file;
}

#undef ENCODE
#undef _FOPEN
#undef _STRING
#undef FOPEN

inline uLong ZCALLBACK __read_file_func(voidpf opaque, voidpf stream, void* buf, uLong size) {
    return (uLong)fread(buf, 1, (size_t)size, (FILE *)stream);
}

inline uLong ZCALLBACK __write_file_func(voidpf opaque, voidpf stream, const void* buf, uLong size) {
    return (uLong)fwrite(buf, 1, (size_t)size, (FILE *)stream);
}

inline long ZCALLBACK __tell_file_func(voidpf opaque, voidpf stream) {
    return ftell((FILE *)stream);
}

static long ZCALLBACK __seek_file_func(voidpf opaque, voidpf stream, uLong offset, int origin) {
    int fseek_origin = 0;
    long ret = 0;

    switch (origin) {
    case ZLIB_FILEFUNC_SEEK_CUR:
        fseek_origin = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END:
        fseek_origin = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET:
        fseek_origin = SEEK_SET;
        break;
    default:
        return -1;
    }
    
    if (fseek((FILE *)stream, offset, fseek_origin) != 0) ret = -1;
    return ret;
}

inline int ZCALLBACK __close_file_func(voidpf opaque, voidpf stream) {
    return fclose((FILE *)stream);
}

inline int ZCALLBACK __error_file_func(voidpf opaque, voidpf stream) {
    return ferror((FILE *)stream);
}

/// file controll functions map
static zlib_filefunc_def __file_func_map = {
    __open_file_func, __read_file_func, __write_file_func, __tell_file_func,
    __seek_file_func, __close_file_func, __error_file_func, nullptr
};

extern "C" {
    __export zip_archiver_t *zip_open(const char *filename, const char *mode, unsigned short compresslevel) {
        unsigned short type;
        if (strcmp(mode, "w") == 0) type = 0;
        else if (strcmp(mode, "w+") == 0) type = 1;
        else if (strcmp(mode, "a") == 0) type = 2;
        else return nullptr;

        unsigned long handler = (unsigned long)zipOpen2(filename, type, nullptr, &__file_func_map);
        return 0 == handler ? nullptr : new zip_archiver_t { handler, compresslevel };
    }

    __export void zip_close(zip_archiver_t *self, const char *comment) {
        if (!self) return;
        if (self->handler) zipClose((zipFile)self->handler, comment);
        delete self;
        self = nullptr;
    }

    __export bool zip_append(zip_archiver_t *self, const char *data, size_t datasize, const char *dest_filename, const char *password, const char *comment) {
        if (!self || !self->handler) return false;

        zip_fileinfo info;
        std::string name = dest_filename;
        
        __get_datetime(&info.tmz_date);
        info.dosDate = __get_dosdate(info.tmz_date.tm_year, info.tmz_date.tm_mon, info.tmz_date.tm_mday);
        info.internal_fa = 0;
        info.external_fa = 0;
        // supports UTF-8: flagBase = 1<<11
        if (ZIP_OK != zipOpenNewFileInZip4((zipFile)self->handler,
            __exchange_path(name).c_str(), &info, nullptr, 0,
            nullptr, 0, comment, Z_DEFLATED, self->level,
            0, 15, 8, Z_DEFAULT_STRATEGY, password,
            get_crc32(data, datasize, 0xffffffff), 36, 1 << 11))
        {
            return false;
        }
        bool result = ZIP_OK == zipWriteInFileInZip((zipFile)self->handler, (const void *)data, datasize);
        zipCloseFileInZip((zipFile)self->handler);
        return result;
    }

    __export bool zip_append_file(zip_archiver_t *self, const char *src_filename, const char *dest_filename, const char *password, const char *comment) {
        if (!self || !self->handler) return false;
            
        std::string data = __get_filecontent(src_filename);
        return zip_append(self, data.c_str(), data.size(), dest_filename, password, comment);
    }


    /// @private get zip data size (supports embedded zip data)
    static size_t get_zipsize(unz_archiver_t *self, const char *filename) {
        if (!self || !self->handler) return 0;

        size_t offset = unz_offset(self); // get zip data offset
        size_t filesize = 0; // total size of files in the zip data
        for (bool flag = unz_locate_first(self); flag; flag = unz_locate_next(self)) {
            // get the local file header size + compressed size
            unz_file_info_t info;
            if (!unz_info(self, &info, nullptr, 0, nullptr, 0)) return 0;
            filesize += info.compressed_size + info.filename_size + info.extra_size + 30;
        }
        
        unz_locate_first(self); // restore local file position to the first
        return __get_filesize(filename) - (offset - filesize);
    }

    __export unz_archiver_t *unz_open(const char *filename) {
        unsigned long handler = (unsigned long)unzOpen2(filename, &__file_func_map);
        if (0 == handler) return nullptr;
        
        unz_archiver_t *self = new unz_archiver_t{ handler, 0 };
        // calculate zip data size
        self->size = get_zipsize(self, filename);
        return self;
    }

    __export void unz_close(unz_archiver_t *self) {
        if (!self) return;
        if (self->handler) unzClose((unzFile)self->handler);
        delete self;
        self = nullptr;
    }

    __export const char *unz_comment(unz_archiver_t *self) {
        static std::string _comment;

        if (!self || !self->handler) return nullptr;

        unz_global_info info;
        if (UNZ_OK != unzGetGlobalInfo((unzFile)self->handler, &info)) return nullptr;

        _comment.clear();
        _comment.resize(info.size_comment + 1);
        return UNZ_OK == unzGetGlobalComment((unzFile)self->handler, (char*)_comment.c_str(), info.size_comment)
            ? _comment.c_str()
            : nullptr;
    }

    __export bool unz_locate_first(unz_archiver_t *self) {
        return self && self->handler ? UNZ_OK == unzGoToFirstFile((unzFile)self->handler) : false;
    }

    __export bool unz_locate_next(unz_archiver_t *self) {
        return self && self->handler ? UNZ_OK == unzGoToNextFile((unzFile)self->handler) : false;
    }

    __export bool unz_locate_name(unz_archiver_t *self, const char *name) {
        std::string filename = name;
        return self && self->handler ? UNZ_OK == unzLocateFile((unzFile)self->handler, __exchange_path(filename).c_str(), 0) : false;
    }

    __export bool unz_info(unz_archiver_t *self, unz_file_info_t *dest, char *filename, size_t filename_size, char *comment, size_t comment_size) {
        if (!self || !self->handler) return false;
        return UNZ_OK == unzGetCurrentFileInfo((unzFile)self->handler, (unz_file_info*)dest, filename, filename_size, nullptr, 0, comment, comment_size);
    }

    __export bool unz_content(unz_archiver_t *self, char *dest, size_t datasize, const char *password) {
        if (!self || !self->handler) return false;
        if (UNZ_OK != unzOpenCurrentFile3((unzFile)self->handler, nullptr, nullptr, 0, password)) return false;
        unzReadCurrentFile((unzFile)self->handler, dest, datasize);
        unzCloseCurrentFile((unzFile)self->handler);
        return true;
    }

    __export bool unz_pos(unz_archiver_t *self, unz_file_pos_t *dest) {
        return self && self->handler ? UNZ_OK == unzGetFilePos((unzFile)self->handler, (unz_file_pos*)dest) : false;
    }

    __export bool unz_locate(unz_archiver_t *self, unz_file_pos_t *pos) {
        return self && self->handler ? UNZ_OK == unzGoToFilePos((unzFile)self->handler, (unz_file_pos*)pos) : false;
    }

    __export size_t unz_offset(unz_archiver_t *self) {
        return self && self->handler ? unzGetOffset((unzFile)self->handler) : 0;
    }


    __export bool unz_rmdata(const char *filename) {
        unz_archiver_t *arc = unz_open(filename);
        if (!arc) return false;
        
        // copy the file data except the zip field
        FILE *fp = fs_fopen(filename, "rb");
        if (!fp) {
            unz_close(arc);
            return false;
        }
        size_t size = __get_filesize(fp) - arc->size;
        char *bin = new char[size];
        fread(bin, 1, size, fp);
        fs_fclose(fp);
        unz_close(arc);

        // overwrite
        if (nullptr == (fp = fs_fopen(filename, "wb"))) {
            delete [] bin;
            return false;
        }
        fwrite(bin, 1, size, fp);
        fs_fclose(fp);
        return true;
    }

    /// @private compress the directory (base)
    static bool __compress(zip_archiver_t *zip, const char *dir, size_t basedir_len, const char *password, const char *root) {
        fs_dirent_t *dirent = fs_opendir(dir);
        
        if (!dirent) return false;
        do {
            std::string name = fs_readdir_name(dirent);
            if (name == "." || name == "..") continue;
            
            std::string path = fs_readdir_path(dirent);
            if (path_isdir(path)) {
                // process recursively
                if (!__compress(zip, path.c_str(), basedir_len, password, root)) {
                    fs_closedir(dirent);
                    return false;
                }
            } else if (path_isfile(path)) {
                // append file into zip
                if (!zip_append_file(zip, path.c_str(), (root + path.substr(basedir_len)).c_str(), password, nullptr)) {
                    fs_closedir(dirent);
                    return false;
                }
            }
        } while (fs_seekdir(dirent));
        fs_closedir(dirent);
        return true;
    }

    __export bool zip_compress(const char *dir, const char *output, unsigned short level, const char *password, const char *mode, const char *root) {
        if (!path_isdir(dir)) return false;

        zip_archiver_t *zip = zip_open(output, mode, level);
        if (!zip) return false;

        bool result = __compress(zip, dir, path_append_slash(dir).size(), password, root);
        zip_close(zip, nullptr);
        return result;
    }

    __export bool unz_uncompress(const char *zip, const char *dir, const char *password) {
        unz_archiver_t *unz = unz_open(zip);
        if (!unz) return false;

        do {
            unz_file_info_t info;
            if (!unz_info(unz, &info, nullptr, 0, nullptr, 0)) {
                _fputs(stdout, "failed to get info");
                unz_close(unz);
                return false;
            }

            // get file name
            std::string filename;
            filename.resize(info.filename_size);
            if (!unz_info(unz, &info, (char *)filename.c_str(), info.filename_size, nullptr, 0)) {
                _fputs(stdout, "failed to get file name");
                unz_close(unz);
                return false;
            }
            
            // read content
            std::string content;
            content.resize(info.uncompressed_size);
            if (!unz_content(unz, (char *)content.c_str(), info.uncompressed_size, password)) {
                _fputs(stdout, "failed to get content");
                unz_close(unz);
                return false;
            }

            // extract to file
            FILE *fp = fs_fopen((path_append_slash(dir) + filename).c_str(), "wb");
            if (!fp) {
                unz_close(unz);
                return false;
            }
            fwrite(content.c_str(), 1, info.uncompressed_size, fp);
            fclose(fp);
        } while (unz_locate_next(unz));
        unz_close(unz);
        return true;
    }
}