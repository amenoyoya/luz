--- filesystem library ---
fs = fs or {}
fs.path = {}

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
void fs_fclose(struct FILE *fp);
void fs_pclose(struct FILE *fp);
int fgetc(struct FILE *fp);
size_t fread(void *buf, size_t size, size_t n, struct FILE *fp);
size_t fwrite(const void *buf, size_t size, size_t n, struct FILE *fp);
int fputc(int c, struct FILE *fp);
int fseek(struct FILE *fp, long offset, int origin);
long int ftell(struct FILE *fp);
int fflush(struct FILE *fp);

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
-- @returns {cdata<path_stat_t*>}
function fs.path.stat(path)
    local stat = ffi.new("path_stat_t")
    ffi.C.path_stat(stat, path)
    return stat
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
    return ffi.C.fs_rmfile(filename)
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


-- File seek from type
fs.seek_from = {
    head = 0,
    cur = 1,
    tail = 2,
}

-- EOF symbol
fs.eof = -1

--- File reader/writer ---
local filerw = {
    -- Get file size
    -- @returns {number}
    size = function (self)
        local cur = self:pos() -- store current position
        self:seek(0, fs.seek_from.tail)
        
        local size = self:pos()
        self:seek(cur, fs.seek_from.head) -- restore to original position
        return size
    end,
    
    -- Read one line
    -- @returns {string}
    readline = function (self)
        local bytes = {}
        local c = self:readchar()
        while c ~= fs.eof do
            if c == 13 --[[ \r ]] then
                self:seek(1, fs.seek_from.cur) -- skip next lf
                break
            elseif c == 10 --[[ \n ]] then
                break
            end
            bytes[#bytes + 1] = c
            c = self:readchar()
        end
        return string.char(unpack(bytes))
    end,
    
    -- Read 1 byte
    -- @returns {number}
    readchar = function (self)
        return ffi.C.fgetc(self.handler)
    end,
    
    -- Read specified size of data
    -- @param {number} size: reading size
    -- @returns {string}
    read = function (self, size)
        local data = ffi.new("char[?]", size + 1)
        return 0 < ffi.C.fread(data, 1, size, self.handler) and ffi.string(data) or ""
    end,
    
    -- Write data
    -- @param {string} data
    -- @param {number} size: writing size (default: data:len())
    -- @returns {number} written size
    write = function (self, data, size)
        return ffi.C.fwrite(ffi.cast("char*", data), 1, size or data:len(), self.handler)
    end,
    
    -- Write 1 byte
    -- @param {number} c: byte-code
    -- @returns {boolean}
    writechar = function (self, c)
        return fs.eof ~= ffi.C.fputc(c, self.handler)
    end,
    
    -- Seek file pointer
    -- @param {number} offset: seeking bytes
    -- @param {numbedr} from: fs.seek_from.* (default: fs.seek_from.head)
    -- @returns {boolean}
    seek = function (self, offset, from)
        return 0 == ffi.C.fseek(self.handler, offset, from or fs.seek_from.head)
    end,
    
    -- Get current file pointer position
    -- @returns {number}
    pos = function (self)
        return tonumber(ffi.C.ftell(self.handler))
    end,
    
    -- Flush file stream
    -- @returns {boolean}
    flush = function (self)
        return 0 == ffi.C.fflush(self.handler)
    end,
}

-- Open file (sopports pipe-mode)
-- @param {string} filename: corrspondence of recursive mkdir if write-mode
-- @param {string} mode: "r" => read-mode (default), "w" => write-mode, "a" => append-mode, "w+" => read+write-mode
--                       If you specify "p" at the beginning of mode, it will open in pipe-mode: "pw", "pr+", etc
--                       If the mode is write-mode, parent directories will be created automatically
-- @returns {filerw|nil}
function fs.open(filename, mode)
    local file = setmetatable({}, {__index = filerw})
    
    mode = mode or "rb"
    -- pipe-mode
    if mode:match"^p" then
        file.close = function (self) ffi.C.fs_pclose(self.handler) end
        file.handler = ffi.gc(ffi.C.fs_popen(filename, mode.sub(2)), ffi.C.fs_pclose)
        return file.handler and file or nil
    end
    -- create parent directory recursively
    if mode:match"^w" then
        fs.mkdir(fs.path.parentdir(filename))
    end
    -- force to open in binary-mode
    if mode:find"b" == nil then
        mode = mode .. "b"
    end
    file.close = function (self) ffi.C.fs_fclose(self.handler) end
    file.handler = ffi.gc(ffi.C.fs_fopen(filename, mode), ffi.C.fs_fclose)
    return file.handler and file or nil
end

-- Read all data in the file
function fs.readfile(filename, size)
    local file = fs.open(filename, "rb")
    local data = file and file:read(size or file:size()) or ""
    file:close()
    return data
end

-- Write data to the file
function fs.writefile(filename, data, size)
    local file = fs.open(filename, "wb")
    local written = file and file:write(data, size) or 0
    file:close()
    return written
end


--- File enumerator ---
local enumerator = {
    -- Close directory
    close = function(self)
        ffi.C.fs_closedir(self.handler)
    end,

    -- Seek to next file / directory
    -- @returns {boolean}
    seek = function(self)
        return ffi.C.fs_seekdir(self.handler)
    end,

    -- Get current file / directory name
    -- @returns {string}
    readname = function(self)
        return ffi.string(ffi.C.fs_readdir_name(self.handler))
    end,

    -- Get current file / directory path
    readpath = function(self)
        return ffi.string(ffi.C.fs_readdir_path(self.handler))
    end,
} 

-- Open directory
-- @returns {eumerator|nil}
function fs.opendir(dir)
    local dirent = setmetatable({}, {__index = enumerator})
    dirent.handler = ffi.gc(ffi.C.fs_opendir(dir), ffi.C.fs_closedir)
    return dirent.handler and dirent or nil
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
