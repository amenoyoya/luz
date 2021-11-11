--- Zip library ---
fs = fs or {}
fs.zip = {} -- zip archiver
fs.unz = {} -- zip extractor

ffi.cdef [[
typedef struct {
    unsigned long  handler;
    unsigned short level;
} zip_archiver_t;

typedef struct {
    unsigned long handler;
    size_t size;
} unz_archiver_t;

typedef struct {
    unsigned long sec, min, hour, day, month, year;
} datetime_t;

typedef struct {
    unsigned long   version,
                    needed_version,
                    flag,
                    compression_method,
                    dos_date,
                    crc32,
                    compressed_size,
                    uncompressed_size,
                    filename_size,
                    extra_size,
                    comment_size,
                    disknum_start,
                    internal_attr,
                    external_attr;
    datetime_t      created_at;
} unz_file_info_t;

typedef struct {
    unsigned long   pos_in_zip,
                    num_of_file;
} unz_file_pos_t;

zip_archiver_t *zip_open(const char *filename, const char *mode, unsigned short compresslevel);
void zip_close(zip_archiver_t *self, const char *comment);
bool zip_append(zip_archiver_t *self, const char *data, size_t datasize, const char *dest_filename, const char *password, const char *comment);
bool zip_append_file(zip_archiver_t *self, const char *src_filename, const char *dest_filename, const char *password, const char *comment);
unz_archiver_t *unz_open(const char *filename);
void unz_close(unz_archiver_t *self);
const char *unz_comment(unz_archiver_t *self);
bool unz_locate_first(unz_archiver_t *self);
bool unz_locate_next(unz_archiver_t *self);
bool unz_locate_name(unz_archiver_t *self, const char *name);
bool unz_info(unz_archiver_t *self, unz_file_info_t *dest, char *filename, size_t filename_size, char *comment, size_t comment_size);
bool unz_content(unz_archiver_t *self, char *dest, size_t datasize, const char *password);
bool unz_pos(unz_archiver_t *self, unz_file_pos_t *dest);
bool unz_locate(unz_archiver_t *self, unz_file_pos_t *pos);
size_t unz_offset(unz_archiver_t *self);
]]

--- ZipArchiver ---
local zip_archiver = {
    -- Close the archiver
    -- @param {string} comment: global comment if you want to append
    -- @returns {boolean}
    close = function (self, comment)
        return self.handler and ffi.C.zip_close(self.handler, comment) or false
    end,

    -- Append data into the archiver
    -- @param {string} data: appending data
    -- @param {number} datasize: appending data size
    -- @param {string} dest_filename: file name in the archiver
    -- @param {string} password (default: nil)
    -- @param {string} comment (default: nil)
    -- @returns {boolean}
    append = function (self, data, datasize, dest_filename, password, comment)
        return ffi.C.zip_append(self.handler, data, datasize, dest_filename, password, comment)
    end,

    -- Append file into the archiver
    -- @param {string} src_filename: appending file name
    -- @param {string} dest_filename: file name in the archiver
    -- @param {string} password (default: nil)
    -- @param {string} comment (default: nil)
    -- @returns {boolean}
    append_file = function (self, src_filename, dest_filename, password, comment)
        return ffi.C.zip_append_file(self.handler, src_filename, dest_filename, password, comment)
    end,

    -- Get/Set compress level
    -- @param {number|nil} level: new compress level
    -- @returns {number} current compress level
    compress_level = function (self, level)
        if type(level) == "number" then
            self.handler.level = level
        end
        return self.handler.level
    end,
}

-- Open zip file (archiver)
-- @param {string} filename
-- @param {string} mode:
--         "w": create new file. (default)
--         "w+": embed zip data to the file
--         "a": append data to the zip file
--        * parent directories will be created automatically
-- @param {number} compresslevel: 0(default) - 9
-- @returns {zip_archiver|nil}
function fs.zip.open(filename, mode, compresslevel)
    print(ffi.C.zip_open)
    local archiver = setmetatable(
        {
            handler = ffi.gc(ffi.C.zip_open(filename, mode or "w", compresslevel or 0), ffi.C.zip_close)
        }, {
            __index = zip_archiver
        }
    )
    print(archiver)
    return archiver.handler and archiver or nil
end


--- ZipExtractor ---
local zip_extractor = {
    -- Close the extractor
    close = function (self)
        return self.handler and ffi.C.unz_close(self.handler)
    end,

    -- Get global comment of the zip file
    -- @returns {string}
    comment = function (self)
        return ffi.string(ffi.C.unz_comment(self.handler))
    end,
    
    -- Locate first entry file in the zip file
    -- @returns {boolean}
    locate_first = function (self)
        return ffi.C.unz_locate_first(self.handler)
    end,

    -- Locate next entry file in the zip file
    -- @returns {boolean}
    locate_next = function (self)
        return ffi.C.unz_locate_next(self.handler)
    end,

    -- Locate specified name of file in the zip file
    -- @param {string} name
    -- @returns {boolean}
    locate_name = function (self, name)
        return ffi.C.unz_locate_name(self.handler, name)
    end,

    -- Get current file information in the zip data
    -- @param {boolean} isContentRequired (default: false): if you want get uncompressed file data, designate `true`
    -- @param {string} password (default: nil): if you want get uncompressed file data, designate the password
    -- @returns {table|nil}
    info = function (self, isContentRequired, password)
        local info = ffi.new"unz_file_info_t"
        if not ffi.C.unz_info(self.handler, info, nil, 0, nil, 0) then return nil end

        -- get the file name and comment
        local filename = ffi.new("char[?]", info.filename_size)
        local comment = ffi.new("char[?]", info.comment_size)
        if not ffi.C.unz_info(self.handler, info, filename, info.filename_size, comment, info.comment_size) then return nil end

        -- get the file content
        local content = nil
        if isContentRequired then
            local data = ffi.new("char[?]", info.uncompressed_size)
            if not ffi.C.unz_content(self.handler, data, info.uncompressed_size, password) then return nil end
            content = ffi.string(data)
        end
        
        return {
            version = info.version,
            needed_version = info.needed_version,
            flag = info.flag,
            compression_method = info.compression_method,
            dos_date = info.dos_date,
            crc32 = info.crc32,
            compressed_size = info.compressed_size,
            uncompressed_size = info.uncompressed_size,
            filename_size = info.filename_size,
            extra_size = info.extra_size,
            comment_size = info.comment_size,
            disknum_start = info.disknum_start,
            internal_attr = info.internal_attr,
            external_attr = info.external_attr,
            created_at = info.created_at, -- cdata<datetime_t*>
            filename = ffi.string(filename),
            comment = ffi.string(comment),
            content = content, -- uncompressed data
        }
    end,

    -- Get current file position in the zip data
    -- @returns {cdata<unz_file_pos_t*>|nil}
    pos = function (self)
        local pos = ffi.new"unz_file_pos_t"
        return ffi.C.unz_pos(self.handler, pos) and pos or nil
    end,

    -- Locate to the designated position
    -- @param {cdata<unz_file_pos_t*>} pos
    -- @returns {boolean}
    locate = function (self, pos)
        return ffi.C.unz_locate(self.handler, pos)
    end,

    -- Get the zip file offset (if the data is embedded zip data, return larger than 0)
    -- @returns {number}
    offset = function (self)
        return ffi.C.unz_offset(self.handler)
    end,

    -- Get the zip data size (supports embedded zip data)
    -- @returns {number}
    size = function (self)
        return self.handler.size
    end,
}

-- Open zip file (extractor)
-- @param {string} filename
-- @returns {zip_extractor|nil}
function fs.unz.open(filename)
    local extractor = setmetatable(
        {
            handler = ffi.gc(ffi.C.unz_open(filename), ffi.C.unz_close)
        }, {
            __index = zip_extractor
        }
    )
    return extractor.handler and extractor or nil
end


--- Utility functions ---

-- Remove embedded zip data from the file
-- @param {string} filename
-- @returns {boolean}
function fs.unz.rmdata(filename)
    local arc = fs.unz.open(filename)
    if arc == nil then return false end
    
    -- copy the file data except the zip field
    local file = fs.open(filename, "rb")
    local size = file:size() - arc:size()
    local bin = file:read(size)
    -- overwrite
    file:close()
    file = fs.open(filename, "wb")
    local result = 0 < file:write(bin, size)
    file:close()
    return result
end

-- @private Compress the directory (base)
local function compress(zip, dirPath, baseDirLen, password, rootDir)
    local f = fs.opendir(dirPath)
    
    if f == nil then return false end
    repeat
        if f:name() ~= "." and f:name() == ".." then
            local path = f:path()
            if fs.path.isdir(path) then
                -- process recursively
                if not compress(zip, path, baseDirLen, password, rootDir) then return false end
            elseif fs.path.isfile(path) then
                -- append file into zip
                if not zip:append_file(path, rootDir .. path:sub(baseDirLen + 1), password) then return false end
            end
        end
    until not f:seek()
    f:close()
    return true
end

-- Compress the directory to zip
-- @param {string} dir: source directory path
-- @param {string} output: output zip file path
-- @param {number} level: compress level (default: 0)
-- @param {string|nil} password: zip password (default: nil)
-- @param {string} mode: "w"(default)|"w+"|"a"
-- @param {string} root: root of local file path in the zip (default: "")
-- @returns {boolean}
function fs.zip.compress(dir, output, level, password, mode, root)
    if not fs.path.isdir(dir) then return false end

    print(dir, output, level, password, mode, root)
    local zip = fs.zip.open(output, mode, level)
    if zip == nil then
        print("cannot open zip")
        return false
    end

    -- local result = compress(zip, dir, fs.path.append_slash(dir):len(), password, root and fs.path.append_slash(root) or "")
    -- zip:close()
    -- return result
end

-- Uncompress the zip into directory
-- @param {string} zip: source zip file
-- @param {string} dir: output directory path
-- @param {string|nil} password (default: nil)
-- @returns {boolean}
function fs.unz.uncompress(zip, dir, password)
    local unz = fs.unz.open(zip)
    if unz == nil then return false end
    repeat
        local info = unz:info(true, password)
        if info == nil then return false end
        
        local file = fs.open(fs.path.append_slash(dir) .. info.filename, "wb")
        if file == nil then return false end
        file:write(info.content, info.uncompressed_size)
    until not unz:locate_next()
    unz:close()
    return true
end
