--- UTF-8 string library ---

--[=[
・utf8.escape(str) -> utf8 string
    escape a str to UTF-8 format string.
・utf8.charpos(s[[, charpos], offset]) -> charpos, code point
    convert UTF-8 position to byte offset. if only offset is given, return byte offset of this UTF-8 char index. if charpos and offset is given, a new charpos will calculate, by add/subtract UTF-8 char offset to current charpos. in all case, it return a new char position, and code point (a number) at this position.
・utf8.next(s[, charpos[, offset]]) -> charpos, code point
    iterate though the UTF-8 string s. If only s is given, it can used as a iterator:
        for pos, code in utf8.next, "utf8-string" 
・utf8.width(s[, ambi_is_double[, default_width]]) -> width
    calculate the width of UTF-8 string s. if ambi_is_double is given, the ambiguous width character's width is 2, otherwise it's 1. fullwidth/doublewidth character's width is 2, and other character's width is 1. if default_width is given, it will be the width of unprintable character, used display a non-character mark for these characters. if s is a code point, return the width of this code point.
・utf8.widthindex(s, location[, ambi_is_double[, default_width]]) -> idx, offset, width
    return the character index at given location in string s. this is a reverse operation of utf8.width(). this function return a index of location, and a offset in in UTF-8 encoding. e.g. if cursor is at the second column (middle) of the wide char, offset will be 2. the width of character at idx is returned, also.
・utf8.title(s) -> new_string
・utf8.fold(s) -> new_string
    convert UTF-8 string s to title-case, or folded case used to compare by ignore case. if s is a number, it's treat as a code point and return a convert code point (number). utf8.lower/utf8.upper has the same extension.
・utf8.ncasecmp(a, b) -> [-1,0,1]
    compare a and b without case, -1 means a < b, 0 means a == b and 1 means a > b.
]=]

-- Get utf8 byte codes of [i] - [j] characters
function string:u8byte(i, j)
    return utf8.byte(self, i, j)
end

-- Create utf8 string from byte codes
function string:u8char(...)
    return utf8.char(self, ...)
end

-- Search pattern and return matched head index + tail index
-- if the capture pattern is designated, return captured strings after arguments 3...
-- @param init: search start index (default: 1)
-- @param plain: if you designate `true`, regular expression will be not used (default: false)
function string:u8find(pattern, init, plain)
    return utf8.find(self, pattern, init, plain)
end

-- Iterate strings that match the pattern (use in for-sequence)
function string:u8gmatch(pattern)
    return utf8.gmatch(self, pattern)
end

-- Replace the pattern into `repl` (string/table/function)
-- @param n: replacing count (default: all)
function string:u8gsub(pattern, repl, n)
    return utf8.gsub(self, pattern, repl, n)
end

-- Get the string length (not bytes)
function string:u8len()
    return utf8.len(self)
end

-- Convert the string to lower-case
function string:u8lower()
    return utf8.lower(self)
end

-- Get the part of string that matches the pattern
-- @param init: search start index (default: 1)
function string:u8match(pattern, init)
    return utf8.match(self, pattern, init)
end

-- Get the reversed string
function string:u8reverse()
    return utf8.reverse(self)
end

-- Get the substring of [i] - [j]
function string:u8sub(i, j)
    return utf8.sub(self, i, j)
end

-- Get the substring of [i] - [i + n]
function string:u8substr(i, n)
    return utf8.sub(self, i, i-1 + (n or utf8.len(self)-i))
end

-- Convert the string to upper-case
function string:u8upper()
    return utf8.upper(self)
end

-- Insert substring into the before of index (default: last)
function string:u8insert(index, substring)
    return utf8.insert(self, index, substring)
end

-- Remove [start](default: -1) - [stop](default: -1)
function string:u8remove(start, stop)
    return utf8.remove(self, start, stop)
end

-- Get byte size of the character byte code
-- @returns 1 - 4 | nil: not utf8 byte code
function utf8.bsize(c)
    if c <= 0x7f then
        return 1
    elseif c >= 0xc2 and c <= 0xdf then
        return 2
    elseif c >= 0xe0 and c <= 0xef then
        return 3
    elseif c >= 0xf0 and c <= 0xf7 then
        return 4
    end
end


-- ffi functions
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
        ffi.C.io_setu16mode(fp)
    end
end

-- UTF-8 string to wide string (UTF-16)
function string:u8towcs()
    local size = self:len()
    local dest = ffi.new("wchar_t[?]", size + 1) -- +1 buffer for the end of null pointer
    return ffi.C.u8towcs(dest, self, size) and dest or nil
end

-- Wide string (UTF-16) to UTF-8 string
function string.wcstou8(src)
    local size = ffi.C.wcslen(src) * 3 -- 1 char of utf8: max 3 byte
    local dest = ffi.new("char[?]", size + 1) -- +1 buffer for the end of null pointer
    return ffi.C.wcstou8(dest, src, size) and ffi.string(dest) or nil
end

-- Flag for serialization when printing table
table.print_flag = true -- true: print(tbl) => table.serialize(tbl, 2, true)

-- override default `print` function
-- * support UTF-8 for Windows
function print(...)
    local list = {...}
    local n = #list
    if n == 0 then
        ffi.C.fputws(string.u8towcs("nil"), io.stdout)
    else
        for i = 1, n do
            ffi.C.fputws(
                string.u8towcs(
                    (i == 1 and "" or "\t") .. (
                        (type(list[i]) == 'table' and table.print_flag) and table.serialize(list[i], 2, true)
                        or (list[i] == nil and "nil" or tostring(list[i]))
                    )
                ),
                io.stdout
            )
        end
    end
    ffi.C.fputws(string.u8towcs("\n"), io.stdout)
end

-- Print formatted string
function printf(format, ...)
    ffi.C.fputws(format:format(...):u8towcs(), io.stdout)
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
    local buf = ffi.new("wchar_t[?]", size + 1)
    return ffi.string(string.wcstou8(ffi.C.fgetws(buf, size, io.stdin))):remove() -- remove the end of line
end
