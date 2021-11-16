/// lua core libraries
const char *corelib = R"(
--- Type checker ---

-- @private Check type is same
-- @param {string} typename
-- @param {string} types: If you want to check for multiple types, designate like following: "string|number|boolean"
--                        If you want to skip the check, designate "any"
local function typesame(typename, types)
    for t in types:gmatch"[^|]+" do
        if t == "any" or typename == t then return true end
    end
    return false
end

-- Type checker for function arguments
-- @param {number} n: argument count
function debug.checkargs(n, ...)
    local funcname = debug.getinfo(2, "n").name
    local args = {...}
    -- ipairs can't scan all elements if the list is like {1, nil, 3} (scan only 1)
    for i = 1, n * 2 - 1, 2 do
        local t = type(args[i])
        if not typesame(t, args[i + 1]) then
            error(string.format(
                "function argument type error: '%s' argument %d expected %s, but got %s",
                funcname,
                math.floor((i - 1) / 2) + 1,
                args[i + 1],
                t
            ))
        end
    end
end


--- Table serializer ---
local escape_char_map = {
    [ "\\" ] = "\\",
    [ "\"" ] = "\"",
    [ "\b" ] = "b",
    [ "\f" ] = "f",
    [ "\n" ] = "n",
    [ "\r" ] = "r",
    [ "\t" ] = "t",
}

local function escape_char(c)
    return "\\" .. (escape_char_map[c] or string.format("u%04x", c:byte()))
end

-- @private Process of escaping
local function escape_string(str)
    return str:gsub('[%z\1-\31\\"]', escape_char)
end

-- @private Stringify Lua value
-- @param {boolean} all: all values will be stringified if `true`
local function value2str(v, all)
    local vt = type(v)
    
    if vt == 'nil' then
        return "nil"
    elseif vt == 'number' then
        return tostring(v)
    elseif vt == 'string' then
        return '"' .. escape_string(v) .. '"'
    elseif vt == 'boolean' then
        return tostring(v)
    elseif vt == 'table' then
        return tostring(v)
    elseif all then
        return tostring(v)
    end
    return ""
end

-- @private Stringify table's field
-- if field is number: return ''
-- if field is not number: return '["$field"]='
local function field2str(v)
    if type(v) == 'number' then return "" end
    return '["' .. escape_string(v) .. '"]='
end

-- @private Basic serialization
-- @param {table} tbl
-- @param {boolean} all: all values will be stringified recursively if `true`
-- @param {number} space_count: indent space count
-- @param {string} space: current indent space count
local function serialize_table(tbl, all, space_count, space)
    local buf = ""
    local f, v = next(tbl, nil)
    local next_space = space

    if space_count > 0 then
        next_space = space .. (" "):rep(space_count)
    end
    
    while f do
        if type(v) == 'table' then
            if buf ~= "" then
                buf = buf .. "," .. next_space
            end
            buf = buf.. field2str(f) .. (all and serialize_table(v, all, space_count, next_space) or value2str(v))
        else
            local value = value2str(v, all)
            if value then
                if buf ~= "" then
                    buf = buf .. "," .. next_space
                end
                buf = buf .. field2str(f) .. value
            end
        end
        -- next
        f, v = next(tbl, f)
    end

    return '{' .. next_space .. buf .. space .. '}'
end

-- Serialize the table
-- @param {number} space_count (default: 0): indent space count
-- @param {boolean} all (default: false): all values will be stringified if `true`
function table:serialize(space_count, all)
    if type(self) ~= 'table' then
        return value2str(self, all) -- not table
    end

    space_count = space_count == nil and 0 or space_count
    all = all or false
    return serialize_table(self, all, space_count, space_count > 0 and "\n" or "")
end


--- String replacer ---
-- @private Replace the part of [head] - [tail] into `repl` string
-- @param {string} str: Target string
-- @param {number} head
-- @param {number} tail
-- @param {string} repl: Replaced to this string
-- @param {function} sub: string.sub | string.u8sub 
local function replacestr(str, head, tail, repl, sub)
    return sub(str, 1, head - 1) .. repl .. sub(str, tail + 1)
end

-- @private Replace the matched `old` string into `new` string
-- @param {number} start (default: 1): Search start index
-- @param {boolean} usepatt (default: false): Is required to use pattern matching
-- @param {function} find: string.find | string.u8find 
-- @param {function} sub: string.sub | string.u8sub 
local function updatestr(str, old, new, start, usepatt, find, sub)
    local head, tail = find(str, old, start or 1, not (usepatt))
    if head and tail then return replacestr(str, head, tail, new, sub) end
    return str
end

-- Replace string
-- @usage string:replace(head number, tail number, repl string) -> string: Replace the part of [head] - [tail] into `repl` string
-- @usage string:replace(old string, new string, start number, usepatt boolean) -> string: Replace the matched `old` string into `new` string
function string:replace(var1, var2, var3, var4)
    if type(var1) == 'number' and type(var2) == 'number' then
        return replacestr(self, var1, var2, var3, string.sub)
    end
    if type(var1) == 'string' then
        return updatestr(self, var1, var2, var3, var4, string.find, string.sub)
    end
    error[[
function 'string.replace' requires arguments
(string, number, number, string)\n\tor\n(string, string, string [,number, boolean])
    ]]
end


--- String + IO FFI functions ---
ffi.cdef[[
bool u8towcs(wchar_t *dest, const char *source, size_t size);
bool wcstou8(char *dest, const wchar_t *source, size_t size);
size_t wcslen(const wchar_t *str);
int fputws(const wchar_t *str, struct FILE *fp);
wchar_t *fgetws(wchar_t *dest, size_t n, struct FILE *fp);
]]

-- UTF-8 string to wide string (UTF-16)
function string:u8towcs()
    debug.checkargs(1, self, "string")
    local dest = reco.new(self:len() * ffi.sizeof"wchar_t")
    return ffi.C.u8towcs(dest:cast"wchar_t*", self, dest.size) and dest:cast"const wchar_t*" or nil
end

-- Wide string (UTF-16) to UTF-8 string
function string.wcstou8(src)
    debug.checkargs(1, src, "cdata|userdata")
    local dest = reco.new(ffi.C.wcslen(src) * 3) -- max 3 byte in utf-8 1 char
    return ffi.C.wcstou8(dest:cast"char*", src, dest.size) and dest:tostr() or ""
end

-- @private General print function
-- * support UTF-8 for Windows
-- * support to print nil
local function fprint(stdout, ...)
    local list = {...} -- #list returns 1 if the list is like {1, nil, 3}
    local key = 0
    -- ipairs can't scan all elements if the list is like {1, nil, 3} (scan only 1)
    for k, v in pairs(list) do
        if key > 0 then ffi.C.fputws(string.u8towcs"\t", stdout) end
        if k > key + 1 then
            for i = 1, k - key - 1 do
                ffi.C.fputws(string.u8towcs"nil\t", stdout)
            end
        end
        ffi.C.fputws(
            string.u8towcs(
                type(list[k]) == 'table' and table.serialize(list[k], 2, true) or tostring(list[k])
            ),
            stdout
        )
        key = k
    end
    ffi.C.fputws(string.u8towcs((key == 0 and "nil" or "") .. "\n"), stdout)
end

-- Override default `print` function
function print(...)
    fprint(io.stdout, ...)
end

-- Print formatted string
function printf(format, ...)
    ffi.C.fputws(format:format(...):u8towcs(), io.stdout)
end

-- Error print function
function eprint(...)
    fprint(io.stderr, ...)
end

function eprintf(format, ...)
    ffi.C.fputws(format:format():u8towcs(), io.stderr)
end

-- Print formatted string to stderr
function errorf(format, ...)
    return error(format:format(...))
end

-- Read string from stdin (support UTF-8 for Windows)
function readln(message, size)
    if type(message) == 'string' then
        ffi.C.fputws(message:u8towcs(), io.stdout)
    end

    size = size or 1024
    local buf = reco.new(size * ffi.sizeof"wchar_t")
    return string.wcstou8(ffi.C.fgetws(buf:cast"wchar_t*", size, io.stdin)):remove() -- remove the end of line
end
)";

const char *fslib = R"(
--- File System ---
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
bool path_stat(path_stat_t *dest, const char *path);
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
    debug.checkargs(1, path, "string")
    local dest = reco.new(path:len())
    return ffi.C.path_basename(dest:cast"char*", path) and dest:tostr() or ""
end

-- Get the base name of path (without extension)
-- @param {string} path: e.g. "/path/to/sample.txt" => "sample"
function fs.path.stem(path)
    debug.checkargs(1, path, "string")
    local dest = reco.new(path:len())
    return ffi.C.path_stem(dest:cast"char*", path) and dest:tostr() or ""
end

-- Get the extension of path
-- @param {string} path: e.g. "/path/to/sample.txt" => ".txt"
function fs.path.ext(path)
    debug.checkargs(1, path, "string")
    local dest = reco.new(path:len())
    return ffi.C.path_ext(dest:cast"char*", path) and dest:tostr() or ""
end

-- Get the parent directory of path
-- @param {string} path: e.g. "/path/to/sample.txt" => "/path/to"
function fs.path.parentdir(path)
    debug.checkargs(1, path, "string")
    local dest = reco.new(path:len())
    return ffi.C.path_parentdir(dest:cast"char*", path) and dest:tostr() or ""
end

-- Identifies if the path is file
function fs.path.isfile(path)
    debug.checkargs(1, path, "string")
    return ffi.C.path_isfile(path)
end

-- Identifies if the path is directory
function fs.path.isdir(path)
    debug.checkargs(1, path, "string")
    return ffi.C.path_isdir(path)
end

-- Get the full path
function fs.path.complete(path)
    debug.checkargs(1, path, "string")
    local dest = reco.new("char[?]", 1024)
    return ffi.C.path_complete(dest:cast"char*", path) and dest:tostr() or ""
end

-- Get the file / directory state
-- @returns {table<path_stat_t>|nil}
function fs.path.stat(path)
    debug.checkargs(1, path, "string")
    local stat = ffi.new("path_stat_t")
    if not ffi.C.path_stat(stat, path) then return nil end
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
    debug.checkargs(1, path, "string")
    local dest = reco.new(path:len() + 1)
    return ffi.C.path_append_slash(dest:cast"char*", path) and dest:tostr() or ""
end

-- Remove slash symbol from the end of path
function fs.path.remove_slash(path)
    debug.checkargs(1, path, "string")
    local dest = reco.new(path:len())
    return ffi.C.path_remove_slash(dest:cast"char*", path) and dest:tostr() or ""
end

-- Copy file
function fs.copyfile(src, dest, isOverwrite)
    debug.checkargs(2, src, "string", dest, "string")
    return ffi.C.fs_copyfile(src, dest, isOverwrite == nil and true or isOverwrite)
end

-- Remove file
function fs.rmfile(filename)
    debug.checkargs(1, filename, "string")
    return ffi.C.fs_rmfile(filename)
end

-- Create directory (recursively)
function fs.mkdir(dir)
    debug.checkargs(1, dir, "string")
    return ffi.C.fs_mkdir(dir)
end

-- Copy directory
function fs.copydir(src, dest)
    debug.checkargs(2, src, "string", dest, "string")
    return ffi.C.fs_copydir(src, dest)
end

-- Remove directory
function fs.rmdir(dir)
    debug.checkargs(1, dir, "string")
    return ffi.C.fs_rmdir(dir)
end

-- Rename (Move) file / directory
function fs.rename(src, dest, isOverwrite)
    debug.checkargs(2, src, "string", dest, "string")
    return ffi.C.fs_rename(src, dest, isOverwrite == nil and true or isOverwrite)
end


-- File seek from type
fs.seekfrom = {
    head = 0,
    cur = 1,
    tail = 2,
}

-- EOF symbol
fs.eof = -1

--- File reader/writer ---
local file_t = {
    -- Close the file
    close = function (self)
        self.reco:close()
        self.handler = nil
    end,

    -- Read 1 byte
    -- @returns {number}
    readchar = function (self)
        return self.handler and ffi.C.fgetc(self.handler) or fs.eof
    end,
    
    -- Read one line
    -- @returns {string}
    readline = function (self)
        if self.handler == nil then return "" end

        local bytes = {}
        local c = self:readchar()
        while c ~= fs.eof do
            if c == 13 --[[ \r ]] then
                self:seek(1, fs.seekfrom.cur) -- skip next lf
                break
            elseif c == 10 --[[ \n ]] then
                break
            end
            bytes[#bytes + 1] = c
            c = self:readchar()
        end
        return string.char(unpack(bytes))
    end,
    
    -- Read specified size of data
    -- @param {number} size: reading size
    -- @returns {reco.record|nil}
    read = function (self, size)
        debug.checkargs(1, size, "number")
        local data = reco.new(size)
        return (self.handler and size == ffi.C.fread(data:cast"void*", 1, size, self.handler)) and data or nil
    end,
    
    -- Write data
    -- @param {string} data
    -- @param {number} size: writing size (default: strlen(data))
    -- @returns {number} written size
    write = function (self, data, size)
        debug.checkargs(1, data, "string")
        return self.handler and ffi.C.fwrite(ffi.cast("const char*", data), 1, size or data:len(), self.handler) or 0
    end,
    
    -- Write 1 byte
    -- @param {number} c: byte-code
    -- @returns {boolean}
    writechar = function (self, c)
        debug.checkargs(1, c, "number")
        return self.handler and fs.eof ~= ffi.C.fputc(c, self.handler) or false
    end,
    
    -- Seek file pointer
    -- @param {number} offset: seeking bytes
    -- @param {numbedr} from: fs.seekfrom.* (default: fs.seekfrom.head)
    -- @returns {boolean}
    seek = function (self, offset, from)
        debug.checkargs(1, offset, "number")
        return self.handler and 0 == ffi.C.fseek(self.handler, offset, from or fs.seekfrom.head) or false
    end,
    
    -- Get current file pointer position
    -- @returns {number}
    pos = function (self)
        return self.handler and tonumber(ffi.C.ftell(self.handler)) or fs.eof
    end,
    
    -- Flush file stream
    -- @returns {boolean}
    flush = function (self)
        return self.handler and 0 == ffi.C.fflush(self.handler) or false
    end,
}

-- Open file (sopports pipe-mode)
-- @param {string} filename: corrspondence of recursive mkdir if write-mode
-- @param {string} mode: "r" => read-mode (default), "w" => write-mode, "a" => append-mode, "w+" => read+write-mode
--                       If you specify "p" at the beginning of mode, it will open in pipe-mode: "pw", "pr+", etc
--                       If the mode is write-mode, parent directories will be created automatically
-- @returns {file_t|nil}
function fs.open(filename, mode)
    debug.checkargs(1, filename, "string")
    mode = mode or "rb"
    local self = { unpack(file_t) }
    -- pipe-mode
    if mode:match"^p" then
        self.reco = reco.record.new(
            function ()
                return ffi.cast("unsigned long", ffi.C.fs_popen(filename, mode.sub(2))), 0
            end,
            function (fp)
                if fp ~= 0 then ffi.C.fs_pclose(ffi.cast("struct FILE*", fp)) end
            end
        )
        self.handler = self.reco:cast"struct FILE*"
        return self.handler and self or nil
    end
    -- create parent directory recursively
    if mode:match"^w" then
        fs.mkdir(fs.path.parentdir(filename))
    end
    -- force to open in binary-mode
    if mode:find"b" == nil then
        mode = mode .. "b"
    end
    self.reco = reco.record.new(
        function ()
            local fp, size = ffi.C.fs_fopen(filename, mode), 0
            if fp == nil then return 0, 0 end
            -- Get file size
            ffi.C.fseek(fp, 0, fs.seekfrom.tail)
            size = tonumber(ffi.C.ftell(fp))
            ffi.C.fseek(fp, 0, fs.seekfrom.head) -- restore file pointer position
            return ffi.cast("unsigned long", fp), size
        end,
        function (fp)
            if fp ~= 0 then ffi.C.fs_fclose(ffi.cast("struct FILE*", fp)) end
        end
    )
    self.handler = self.reco:cast"struct FILE*"
    return self.handler and self or nil
end

-- Read all data in the file
function fs.readfile(filename, size)
    debug.checkargs(1, filename, "string")

    local file, data = fs.open(filename, "rb")
    if file == nil then return "" end
    
    size = size or file.size
    data = file:read(size)
    file:close()
    return data:tostr()
end

-- Write data to the file
function fs.writefile(filename, data, size)
    debug.checkargs(2, filename, "string", data, "string")

    local file, written = fs.open(filename, "wb")
    if file == nil then return "" end

    written = file:write(data, size)
    file:close()
    return written
end


--- File enumerator ---
local dirent_t = {
    -- Close directory
    close = function (self)
        self.reco:close()
        self.handler = nil
    end,

    -- Seek to next file|directory
    -- @returns {boolean}
    next = function (self)
        return self.handler and ffi.C.fs_seekdir(self.handler) or false
    end,

    -- Get current file|directory name
    -- @returns {string}
    readname = function (self)
        return self.handler and ffi.string(ffi.C.fs_readdir_name(self.handler)) or ""
    end,

    -- Get current file|directory path
    readpath = function (self)
        return self.handler and ffi.string(ffi.C.fs_readdir_path(self.handler)) or ""
    end,
} 

-- Open directory
-- @param {string} dir
-- @returns {dirent_t|nil}
function fs.opendir(dir)
    debug.checkargs(1, dir, "string")
    local self = { unpack(dirent_t) }
    self.reco = reco.record.new(
        function ()
            return ffi.cast("unsigned long", ffi.C.fs_opendir(dir)), 0
        end,
        function (dirent)
            if dirent ~= 0 then ffi.C.fs_closedir(ffi.cast("dirent_t*", dirent)) end
        end
    )
    self.handler = self.reco:cast"dirent_t*"
    return self.handler and self or nil
end

-- Scan directory
-- @param {string} dir
-- @param {function(name string, path string, ...)->boolean} callback
-- @param {...} extra arguments for the callback function
-- @returns {boolean}
function fs.scandir(dir, callback, ...)
    debug.checkargs(2, dir, "string", callback, "function")
    local dirent = fs.opendir(dir)
    if dirent == nil then return false end
    repeat
        if not callback(dirent:readname(), dirent:readpath(), ...) then
            dirent:close()
            return false
        end
    until not dirent:next()
    dirent:close()
    return true
end

-- @private Enumerate files|directories base function
local function enumfiles(name, path, dest, nest, mode)
    if name == ".." or name == "." then return true end
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
        return nest ~= 0 and fs.scandir(info.path, enumfiles, dest, nest - 1, mode) or true
    end
    return true
end

-- Enumerate files|directories in the directory
-- @param {string} dir: target directory
-- @param {number} nest: recursive enumeration nest (default: -1 => infite), 0 => no enumeration
-- @param {string} mode: "all"(default) | "file" | "dir"
-- @returns {table[]} {name: string, path: string, isfile: boolean, isdir: boolean}[]
function fs.enumfiles(dir, nest, mode)
    local files = {}
    return fs.scandir(dir, enumfiles, files, nest == nil and -1 or nest, mode or "all") and files or {}
end
)";

const char *syslib = R"(
--- Overload lua standard load function ---

-- Lua original functions store
reco.lua = {
    load = load,
    loadfile = loadfile,
    dofile = dofile,
    require = require,
}

-- Disable `loadstring`
loadstring = nil

-- Override `load`
-- * Supports to get current script info
-- @param {string} code
-- @param {string} chunkname
-- @param {string} mode (default: "bt"): {"b": binary mode, "t": text mode, "bt": binary + text mode}
-- @param {table} env (default: _ENV)
-- @returns {function, string} loader, error_message
function load(code, chunkname, mode, env)
    local f, err = reco.lua.load(code, chunkname, mode, env)
    if f == nil then
        return f, err
    end
    return function()
        -- store previous package.__file, package.__dir
        local __file = package.__file
        local __dir = package.__dir
        -- enable to get current script info from package.__file, package.__dir
        package.__file = chunkname and (chunkname:sub(1, 1) == "@" and chunkname:sub(2) or chunkname)
        package.__dir = package.__file and fs.path.parentdir(package.__file)
        
        local result = f()
        -- restore package.__file, package.__dir
        package.__file = __file
        package.__dir = __dir

        return result
    end, err
end

-- override loadfile
-- @param {string} filename
--     - `*.lua`: load as a lua code
--     - `*.sym`: load as a compiled lua byte code (mode "b" must be set)
-- @param {string} mode (default: "bt")
-- @param {table} env (default: _ENV)
-- @returns {function|nil, string} loader, error_message
function loadfile(filename, mode, env)
    local ext = fs.path.ext(filename)
    -- load lua (plain source code or compiled byte-code)
    return load(fs.readfile(filename), "@" .. filename, mode, env)
end

-- override dofile
-- @param {string} filename
function dofile(filename)
    local f, err = loadfile(filename)
    if not f then
        error(err)
    end
    return f()
end


--- extended package.loaders ---

-- override require
-- * support for requiring relative package
-- * <module_name> will be changed into full path if <module_name> has "/"
function require(module_name)
    local module = reco.lua.require(module_name)
    -- remove package.loaded[module_name] if <module_name> has "/"
    -- => always search package from relative path
    if module_name:find"/" then
        package.loaded[module_name] = nil
    end
    return module
end

-- Append relative module loader
local function try_search_module(filepath, err)
    if fs.path.isfile(filepath) then
        local loader, err = loadfile(filepath)
        if loader == nil then error(err) end
        return loader, err
    end
    return nil, err .. "\n\tno file '" .. filepath .. "'"
end

table.insert(package.loaders, 1, function (module_name)
    local err = ""
    -- search from relative path if <module_name> has "/"
    -- * in this case: "." not replaced into "/"
    if module_name:find"/" and package.__dir then
        for entry in package.path:gmatch"[^;]+" do
            local filepath = fs.path.complete(package.__dir .. "/" .. entry:replace("?", module_name))
            local loader; loader, err = try_search_module(filepath, err)
            if loader then return loader end
        end
    else
        -- normal search
        module_name = module_name:gsub("%.", "/") -- "." => "/"
        for entry in package.path:gmatch"[^;]+" do
            local filepath = entry:replace("?", module_name)
            local loader; loader, err = try_search_module(filepath, err)
            if loader then return loader end
        end
    end
    return err
end)

-- Append simple loader for C library
-- * support for requiring relative package
-- * dynamic link library entry point:
--      + luaopen_{module_name}
--      + luaopen_module
local function try_search_dynlib(filepath, err)
    if fs.path.isfile(filepath) then
        local loader = package.loadlib(filepath, "luaopen_" .. fs.path.stem(filepath))
        if loader == nil then
            loader = package.loadlib(filepath, "luaopen_module")
        end
        if loader == nil then
            error("entry point function not found in module file '" .. filepath .. "'.")
        end
        return loader, err
    end
    return nil, err .. "\n\tno file '" .. filepath .. "'"
end

table.insert(package.loaders, 2, function (module_name)
    local err = ""

    -- search from relative path if <module_name> has "/"
    -- * in this case: "." not replaced into "/"
    if module_name:find"/" and package.__dir then
        for entry in package.cpath:gmatch"[^;]+" do
            local filepath = fs.path.complete(package.__dir .. "/" .. entry:replace("?", module_name))
            local loader; loader, err = try_search_dynlib(filepath, err)
            if loader then return loader end
        end
    else
        -- normal search
        module_name = module_name:gsub("%.", "/") -- "." => "/"
        for entry in package.cpath:gmatch"[^;]+" do
            local filepath = entry:replace("?", module_name)
            local loader; loader, err = try_search_dynlib(filepath, err)
            if loader then return loader end
        end
    end
    return err
end)
)";

inline bool openscript(sol::state &lua, const char *script, const char *name) {
    auto result = lua.safe_script(script, name);
    if (result.valid()) return true;
    
    sol::error err = result;
    _fputs(stderr, err.what());
    return false;
}