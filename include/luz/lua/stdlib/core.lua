--- Core library ---

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
function debug.checkarg(n, ...)
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

--- FFI functions ---

ffi.cdef[[
void io_setu16mode(struct FILE *fp); // Windows only
bool u8towcs(wchar_t *dest, const char *source, size_t size);
bool wcstou8(char *dest, const wchar_t *source, size_t size);
size_t wcslen(const wchar_t *str);
int fputws(const wchar_t *str, struct FILE *fp);
wchar_t *fgetws(wchar_t *dest, size_t n, struct FILE *fp);
]]

if ffi.os == "Windows" then
    -- Set UTF-16 mode to the file pointer
    function io.setu16mode(fp)
        debug.checkarg(1, fp, "cdata|userdata")
        ffi.C.io_setu16mode(fp)
    end
end

-- UTF-8 string to wide string (UTF-16)
function string:u8towcs()
    debug.checkarg(1, self, "string")
    local size = self:len()
    local dest = ffi.new("wchar_t[?]", size + 1) -- +1 buffer for the end of null pointer
    ffi.C.u8towcs(dest, self, size)
    return dest
end

-- Wide string (UTF-16) to UTF-8 string
function string.wcstou8(src)
    debug.checkarg(1, src, "string")
    local size = ffi.C.wcslen(src) * 3 -- 1 char of utf8: max 3 byte
    local dest = ffi.new("char[?]", size + 1) -- +1 buffer for the end of null pointer
    ffi.C.wcstou8(dest, src, size)
    return ffi.string(dest)
end

-- Flag for serialization when printing table
table.print_flag = true -- true: print(tbl) => table.serialize(tbl, 2, true)

-- @private General print function
-- * support UTF-8 for Windows
-- * support to print nil
local function fprint(stdout, ...)
    local list = {...} -- #list returns 1 if the list is like {1, nil, 3}
    local key = 0
    -- ipairs can't scan all elements if the list is like {1, nil, 3} (scan only 1)
    for k, v in pairs(list) do
        if key > 0 then ffi.C.fputws(string.u8towcs("\t"), stdout) end
        if k > key + 1 then
            for i = 1, k - key - 1 do
                ffi.C.fputws(string.u8towcs("nil\t"), stdout)
            end
        end
        ffi.C.fputws(
            string.u8towcs(
                (type(list[k]) == 'table' and table.print_flag) and table.serialize(list[k], 2, true) or tostring(list[k])
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
    debug.checkarg(1, format, "string")
    ffi.C.fputws(format:format(...):u8towcs(), io.stdout)
end

-- Error print function
function eprint(...)
    fprint(io.stderr, ...)
end

function eprintf(format, ...)
    debug.checkarg(1, format, "string")
    ffi.C.fputws(format:format():u8towcs(), io.stderr)
end

-- Print formatted string to stderr
function errorf(format, ...)
    debug.checkarg(1, format, "string")
    return error(format:format(...))
end

-- Read string from stdin (support UTF-8 for Windows)
function readln(message, size)
    if type(message) == 'string' then
        ffi.C.fputws(message:u8towcs(), io.stdout)
    end

    size = size or 1024
    local buf = ffi.new("wchar_t[?]", size + 1)
    return ffi.string(string.wcstou8(ffi.C.fgetws(buf, size, io.stdin))):remove() -- remove the end of line
end
