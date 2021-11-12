#pragma once

#include "string.hpp"
#include <memory>

extern "C" {
    /// structure of file status
    typedef struct {
        unsigned long device_id, inode;
        unsigned short access_mode;
        short nlinks, user_id, group_id;
        unsigned long special_device_id;
        unsigned long long size, // file size
            last_accessed_seconds, // last accessed time (sec)
            last_modified_seconds, // last modified time (sec)
            last_changed_seconds;  // last file status changed time (sec)
    } path_stat_t;

    /// get file name
    // e.g. "/path/to/sample.txt" => "sample.txt"
    __export const char *path_basename(char *dest, const char *path);

    /// get file name without extension
    // e.g. "/path/to/sample.txt" => "sample"
    __export const char *path_stem(char *dest, const char *path);

    /// get file extension
    // e.g. "/path/to/sample.txt" => ".txt"
    __export const char *path_ext(char *dest, const char *path);

    /// get parent directory
    // e.g. "/path/to/sample.txt" => "/path/to"
    __export const char *path_parentdir(char *dest, const char *path);
    
    /// identifies if the path is file
    __export bool path_isfile(const char *path);

    /// identifies if the path is directory
    __export bool path_isdir(const char *path);

    /// get full path
    __export const char *path_complete(char *dest, const char *path);

    /// get the file / directory status
    __export bool path_stat(path_stat_t *dest, const char *path);

    /// append '/' or '\\' to the end
    __export const char *path_append_slash(char *dest, const char *path);

    /// remove '/' or '\\' from the end
    __export const char *path_remove_slash(char *dest, const char *path);
}

/*** ================================================== ***/
/*** utility functions for C++ ***/

/// get file name
std::string path_basename(const std::string &path);

/// get file name without extension
std::string path_stem(const std::string &path);

/// get file extension
std::string path_ext(const std::string &path);

/// get parent directory
std::string path_parentdir(const std::string &path, bool isFullPathRequired = true);

/// identifies if the path is file
bool path_isfile(const std::string &path);

/// identifies if the path is directory
bool path_isdir(const std::string &path);

/// get full path
std::string path_complete(const std::string &path);

/// get the file / directory status
std::unique_ptr<path_stat_t> path_stat(const std::string &path);

/// append '/' or '\\' to the end
inline std::string path_append_slash(const std::string &path) {
    auto it = path.end() - 1;
    const char *slash =
        #ifdef _WINDOWS
            "\\";
        #else
            "/";
        #endif
    if (*it != '/' && *it != '\\') return std::move(path + slash);
    return std::move(path);
}

/// remove '/' or '\\' from the end
inline std::string path_remove_slash(std::string path) {
    auto it = path.end() - 1;
    if (*it == '/' || *it == '\\') path.erase(it);
    return std::move(path);
}