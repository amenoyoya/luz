--- filesystem library ---
fs = {
    path = {}
}

local ffi = require "ffi"

ffi.cdef[[
typedef struct {
    unsigned long device_id, inode;
    unsigned short access_mode;
    short nlinks, user_id, group_id;
    unsigned long special_device_id;
    unsigned long long size,
        last_accessed_seconds,
        last_modified_seconds,
        last_changed_seconds;
} path_stat_t;

const char *path_basename(char *dest, const char *path);
const char *path_stem(char *dest, const char *path);
const char *path_ext(char *dest, const char *path);
const char *path_parentdir(char *dest, const char *path);
bool path_isfile(const char *path);
bool path_isdir(const char *path);
const char *path_complete(char *dest, const char *path);
void path_stat(path_stat_t *dest, const char *path);
const char *path_append_slash(char *dest, const char *path);
const char *path_remove_slash(char *dest, const char *path);

struct FILE *fs_fopen(const char *filename, const char *mode);
struct FILE *fs_popen(const char *procname, const char *mode);
bool fs_copyfile(const char *src, const char *dest, bool isOverwrite);
bool fs_rmfile(const char *filename);
bool fs_mkdir(const char *dir);
bool fs_copydir(const char *src, const char *dest);
bool fs_rmdir(const char *dir);
bool fs_rename(const char *src, const char *dest, bool isOverwrite);

struct fs_dirent_t *fs_opendir(const char *dir);
void fs_closedir(struct fs_dirent_t *self);
bool fs_seekdir(struct fs_dirent_t *self);
const char *fs_readdir_name(struct fs_dirent_t *self);
const char *fs_readdir_path(struct fs_dirent_t *self);
]]

-- Get the base name of path
-- @param {string} path: e.g. "/path/to/sample.txt" => "sample.txt"
function fs.path.basename(path)
    local dest = ffi.new("char[?]", path:len() + 1)
    return ffi.string(ffi.C.path_basename(dest, path))
end

-- Get the base name of path (without extension)
-- @param {string} path: e.g. "/path/to/sample.txt" => "sample"
function fs.path.stem(path)
    local dest = ffi.new("char[?]", path:len() + 1)
    return ffi.string(ffi.C.path_stem(dest, path))
end

-- Get the extension of path
-- @param {string} path: e.g. "/path/to/sample.txt" => ".txt"
function fs.path.ext(path)
    local dest = ffi.new("char[?]", path:len() + 1)
    return ffi.string(ffi.C.path_ext(dest, path))
end

-- Get the parent directory of path
-- @param {string} path: e.g. "/path/to/sample.txt" => "/path/to"
function fs.path.parentdir(path)
    local dest = ffi.new("char[?]", path:len() + 1)
    return ffi.string(ffi.C.path_parentdir(dest, path))
end

-- Identifies if the path is file
function fs.path.isfile(path)
    return ffi.C.path_isfile(path)
end

-- Identifies if the path is directory
function fs.path.isdir(path)
    return ffi.C.path_isdir(path)
end

-- Get the full path
function fs.path.complete(path)
    local dest = ffi.new("char[?]", 1024)
    return ffi.string(ffi.C.path_complete(dest, path))
end

-- Get the file / directory state
function fs.path.stat(path)
    local stat = ffi.new("path_stat_t")
    ffi.C.path_stat(stat, path)
    return {
        device_id = stat.device_id,
        inode = stat.inode,
        access_mode = stat.access_mode,
        nlinks = stat.nlinks,
        user_id = stat.user_id,
        group_id = stat.group_id,
        special_device_id = stat.special_device_id,
        size = stat.size,
        last_accessed_seconds = stat.last_accessed_seconds,
        last_modified_seconds = stat.last_modified_seconds,
        last_changed_seconds = stat.last_changed_seconds,
    }
end

-- Append slash symbol into the end of path
function fs.path.append_slash(path)
    local dest = ffi.new("char[?]", path:len() + 2)
    return ffi.C.path_append_slash(dest, path)
end

-- Remove slash symbol from the end of path
function fs.path.remove_slash(path)
    local dest = ffi.new("char[?]", path:len() + 1)
    return ffi.C.path_remove_slash(dest, path)
end

-- Copy file
function fs.copyfile(src, dest, isOverwrite)
    return ffi.C.fs_copyfile(src, dest, isOverwrite == nil and true or isOverwrite)
end

-- Remove file
function fs.rmfile(filename)
    return ffi.C.fs_rmdir(filename)
end

-- Create directory (recursively)
function fs.mkdir(dir)
    return ffi.C.fs_mkdir(dir)
end

-- Copy directory
function fs.copydir(src, dest)
    return ffi.C.fs_copydir(src, dest)
end

-- Remove directory
function fs.rmdir(dir)
    return ffi.C.fs_rmdir(dir)
end

-- Rename (Move) file / directory
function fs.rename(src, dest, isOverwrite)
    return ffi.C.fs_rename(src, dest, isOverwrite == nil and true or isOverwrite)
end


--- File enumerator ---

-- Open directory
-- @returns {table|nil} FileEnumerator
function fs.opendir(dir)
    local dirent = ffi.gc(ffi.C.fs_opendir(dir), ffi.C.fs_closedir)
    return dirent and {
        dirent = dirent,

        -- Close directory
        close = function(self)
            ffi.C.fs_closedir(self.dirent)
        end,

        -- Seek to next file / directory
        -- @returns {boolean}
        seek = function(self)
            return ffi.C.fs_seekdir(self.dirent)
        end,

        -- Get current file / directory name
        -- @returns {string}
        readname = function(self)
            return ffi.string(ffi.C.fs_readdir_name(self.dirent))
        end,

        -- Get current file / directory path
        readpath = function(self)
            return ffi.string(ffi.C.fs_readdir_path(self.dirent))
        end,
    } or nil
end

-- @private Enumerate files / directories base function
local function enumfiles(dest, dir, nest, mode)
    if nest == 0 then return dest end
    
    local dirent = fs.opendir(dir)
    if dirent == nil then return dest end

    repeat
        local name = dirent:readname()
        if name ~= ".." and name ~= "." then
            local path = dirent:readpath()
            local info = {
                path = path,
                isfile = fs.path.isfile(path),
                isdir = fs.path.isdir(path),
            }
            if info.isfile then
                if mode ~= "dir" then
                    dest[#dest + 1] = info
                end
            elseif info.isdir then
                if mode ~= "file" then
                    dest[#dest + 1] = info
                end
                enumfiles(dest, info.path, nest - 1, mode)
            end
        end
    until not dirent:seek()

    dirent:close()
    return dest
end

-- Enumerate files / directories in the directory
-- @param {string} dir: target directory
-- @param {number} nest: recursive enumeration nest (default: -1 => infite), 0 => no enumeration
-- @param {string} mode: "all"(default) | "file" | "dir"
function fs.enumfiles(dir, nest, mode)
    local files = {}
    return enumfiles(files, dir, nest == nil and -1 or nest, mode or "all")
end
