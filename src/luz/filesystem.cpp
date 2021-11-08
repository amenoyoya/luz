#include <luz/filesystem.hpp>

#ifdef _WINDOWS
    #include <shlwapi.h>
#else
    #include <sys/stat.h>
    #include <dirent.h>
#endif

extern "C" {
    FILE *fs_fopen(const char *filename, const char *mode) {
        if (*mode == 'w') fs_mkdir(path_parentdir(filename).c_str()); // auto create parent directories
        #ifdef _WINDOWS
            return _wfopen(u8towcs(filename).c_str(), u8towcs(mode).c_str());
        #else
            return fopen(filename, mode);
        #endif
    }

    FILE *fs_popen(const char *procname, const char *mode) {
        #ifdef _WINDOWS
            return _wpopen(u8towcs(procname).c_str(), u8towcs(mode).c_str());
        #else
            return fopen(procname, mode);
        #endif
    }

    bool fs_copyfile(const char *src, const char *dest, bool isOverwrite) {
        if (!isOverwrite && path_isfile(dest)) return false;
        
        FILE *in = fs_fopen(src, "rb"), *out = fs_fopen(dest, "wb");
        if (!in || !out) {
            if (in) fclose(in);
            if (out) fclose(out);
            return false;
        }
        
        bool result = true;
        for (int c = fgetc(in); !feof(in); c = fgetc(in)) {
            if (EOF == fputc(c, out)) {
                result = false;
                break;
            }
        }
        if (in) fclose(in);
        if (out) fclose(out);    
        return true;
    }

    bool fs_rmfile(const char *filename) {
        #ifdef _WINDOWS
            return FALSE != DeleteFileW(u8towcs(filename).c_str());
        #else
            return 0 == unlink(filename);
        #endif
    }

    #ifdef _WINDOWS
        bool fs_mkdir(const char *dir) {
            std::wstring wdir = u8towcs(dir);
            wchar_t *p = (wchar_t*)wdir.c_str();
            unsigned long i = 0;
            // create directories in order from the upper level directory
            while (*p != '\0') {
                if ((*p == '/' || *p == '\\') && i > 0) {
                    std::wstring name = wdir.substr(0, i);
                    if (!PathIsDirectory(name.c_str()) && !CreateDirectory(name.c_str(), nullptr)) return false;
                }
                ++p;
                ++i;
            }
            if (!PathIsDirectory(wdir.c_str()) && !CreateDirectory(wdir.c_str(), nullptr)) return false;
            return true;
        }
    #else
        bool fs_mkdir(const char *_dir) {
            std::string dir = _dir;
            char *p = (char*)dir.c_str();
            unsigned long i = 0;
            // create directories in order from the upper level directory
            while (*p != '\0') {
                if ((*p == '/') && i > 0) {
                    std::string name = dir.substr(0, i);
                    if (!path_isdir(name.c_str()) && 0 != mkdir(name.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)) return false;
                }
                ++p;
                ++i;
            }
            if (!path_isdir(dir.c_str()) && 0 != mkdir(dir.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)) return false;
            return true;
        }
    #endif
    
    bool fs_copydir(const char *src, const char *dest) {
        fs_dirent_t *dirent = fs_opendir(src);
        if (dirent == nullptr) return false;
        if (!path_isdir(dest) && !fs_mkdir(dest)){
            fs_closedir(dirent);
            return false;
        }

        std::string dir = std::move(path_append_slash(dest));
        do {
            if (dirent->current_name != "." && dirent->current_name != "..") {
                if (path_isdir(dirent->current_path)) { // copy directory recursively
                    if (!fs_copydir(dirent->current_path.c_str(), (dir + dirent->current_name).c_str())){
                        fs_closedir(dirent);
                        return false;
                    }
                } else { // copy file
                    if (!fs_copyfile(dirent->current_path.c_str(), (dir + dirent->current_name).c_str(), true)) {
                        fs_closedir(dirent);
                        return false;
                    }
                }
            }
        } while (fs_seekdir(dirent));

        fs_closedir(dirent);
        return true;
    }
    
    /// @private remove empty directory
    inline bool rmdir_empty(const char *dir) {
        #ifdef _WINDOWS
            return FALSE != RemoveDirectory(u8towcs(dir).c_str());
        #else
            return 0 == rmdir(dir);
        #endif
    }
    
    bool fs_rmdir(const char *dir) {
        fs_dirent_t *dirent = fs_opendir(dir);
        if (dirent == nullptr) return false;

        do {
            if (dirent->current_name != "." && dirent->current_name != "..") {
                if (path_isdir(dirent->current_path)) { // remove directory recursively
                    if (!fs_rmdir(dirent->current_path.c_str())) {
                        fs_closedir(dirent);
                        return false;
                    }
                } else { // remove file
                    if (!fs_rmfile(dirent->current_path.c_str())) {
                        fs_closedir(dirent);
                        return false;
                    }
                }
            }
        } while (fs_seekdir(dirent));

        fs_closedir(dirent);
        return rmdir_empty(dir); // remove empty directory
    }

    /*** ================================================== ***/
    /*** file enumerator ***/
    #ifdef _WINDOWS
        fs_dirent_t *fs_opendir(const char *_dir) {
            WIN32_FIND_DATA info;
            std::string dir = std::move(path_append_slash(_dir));
            unsigned long handler = (unsigned long)FindFirstFile((u8towcs(dir) + L"*.*").c_str(), &info);

            if (0 == handler) return nullptr;
            
            std::string name = std::move(wcstou8(info.cFileName));
            return new fs_dirent_t {
                handler, std::move(dir), std::move(name), std::move(dir + name)
            };
        }
        
        void fs_closedir(fs_dirent_t *self) {
            if (self == nullptr) return;
            if (self->handler) {
                FindClose((HANDLE)self->handler);
            }
            delete self;
            self = nullptr;
        }
        
        bool fs_seekdir(fs_dirent_t *self) {
            WIN32_FIND_DATA info;
            
            if (!self->handler) return false;
            if (!FindNextFile((HANDLE)self->handler, &info)) return false;
            self->current_name = std::move(wcstou8(info.cFileName));
            self->current_path = std::move(self->directory + self->current_name);
            return true;
        }
    #else
        fs_dirent_t *fs_opendir(const char *_dir) {
            std::string dir = path_append_slash(_dir);
            unsigned long handler = (unsigned long)opendir(dir.c_str());
            
            if (0 == handler) return nullptr;
            
            struct dirent* dent = readdir((DIR*)handler);
            
            if (!dent) {
                closedir((DIR*)handler);
                return nullptr;
            }
            return new fs_dirent_t {
                handler, std::move(dir), dent->d_name, std::move(dir + dent->d_name)
            };
        }
        
        void fs_closedir(fs_dirent_t *self) {
            if (self == nullptr) return;
            if (self->handler) {
                closedir((DIR*)self->handler);
            }
            delete self;
            self = nullptr;
        }
        
        bool fs_seekdir(fs_dirent_t *self) {
            if (!self->handler) return false;
            
            struct dirent* dent = readdir((DIR*)self->handler);
            if (!dent) return false;
            self->current_name = dent->d_name;
            self->current_path = std::move(self->directory + self->current_name);
            return true;
        }
    #endif

    const char *fs_readdir_name(fs_dirent_t *self) {
        return self->current_name.c_str();
    }

    const char *fs_readdir_path(fs_dirent_t *self) {
        return self->current_path.c_str();
    }
}