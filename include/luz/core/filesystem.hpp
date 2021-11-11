#pragma once

#include "path.hpp"

extern "C" {
    /// open file (supports UTF-8 in Windows)
    __export FILE *fs_fopen(const char *filename, const char *mode);

    /// close file safely
    __export void fs_fclose(FILE *fp);

    /// open pipe (supports UTF-8 in Windows)
    __export FILE *fs_popen(const char *procname, const char *mode);

    /// close pipe safely
    __export void fs_pclose(FILE *fp);

    /// copy file
    __export bool fs_copyfile(const char *src, const char *dest, bool isOverwrite);

    /// remove file
    __export bool fs_rmfile(const char *filename);

    /// create directory (recursively)
    __export bool fs_mkdir(const char *dir);

    /// copy directory (recursively)
    __export bool fs_copydir(const char *src, const char *dest);

    /// remove directory (recursively)
    __export bool fs_rmdir(const char *dir);

    /// rename (move) file / directory
    __export bool fs_rename(const char *src, const char *dest, bool isOverwrite);

    /*** ================================================== ***/
    /*** file enumerator ***/

    /// structure for enumeraing files in directory
    typedef struct {
        unsigned long handler;
        std::string directory,    // opening directory path
                    current_name, // current file / directory name
                    current_path; // current file / directory path
    } fs_dirent_t;

    /// open directory for enumerating files
    __export fs_dirent_t *fs_opendir(const char *dir);

    /// close directory
    __export void fs_closedir(fs_dirent_t *self);

    /// seek directory to next file / directory
    // @return false if no file / directory any more
    __export bool fs_seekdir(fs_dirent_t *self);

    /// get current file / directory name
    __export const char *fs_readdir_name(fs_dirent_t *self);

    /// get current file / directory path
    __export const char *fs_readdir_path(fs_dirent_t *self);
}