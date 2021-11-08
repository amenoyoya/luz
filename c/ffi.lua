local ffi = require "ffi"

ffi.cdef[[
bool u8towcs(wchar_t *dest, const char *source, size_t size);
bool wcstou8(char *dest, const wchar_t *source, size_t size);
struct FILE *_wfopen(const wchar_t *filename, const wchar_t *mode);
struct FILE *fopen(const char *filename, const char *mode);
void fclose(struct FILE *fp);
size_t fwprintf(struct FILE *fp, const wchar_t *format, ...);
size_t fread(void *buf, size_t size, size_t n, struct FILE *fp);
void setu16mode(struct FILE *fp);
]]

if ffi.os == "Windows" then
    -- override `print` (unicode correspondence)
    ffi.C.setu16mode(io.stdout)
    function print(...)
        for index, val in ipairs{...} do
            ffi.C.fwprintf(io.stdout, L((index == 1 and "" or "\t") .. tostring(val)))
        end
        ffi.C.fwprintf(io.stdout, L"\n")
    end
end

function L(str)
    local wstr = ffi.new("wchar_t[?]", str:len() + 1) -- +1 buffer for the last null char
    ffi.C.u8towcs(wstr, str, str:len())
    return wstr
end

function U(wstr)
    local str = ffi.new("char[?]", ffi.sizeof(wstr) + 1) -- +1 buffer for the last null char
    ffi.C.wcstou8(str, wstr, ffi.sizeof(wstr))
    return ffi.string(str)
end

function fopen(filename, mode)
    if ffi.os == "Windows" then
        return ffi.gc(ffi.C._wfopen(filename, mode), function(fp) print("fclose", fp); ffi.C.fclose(fp) end)
    end
    return ffi.gc(ffi.C.fopen(U(filename), U(mode)), function(fp) print("fclose", fp); ffi.C.fclose(fp) end)
end

function fwrite(fp, format, ...)
    return ffi.C.fwprintf(fp, format, ...)
end

function fread(fp, n)
    local buf = ffi.new("char[?]", n + 1) -- +1 buffer for the last null char
    ffi.C.fread(buf, 1, n, fp)
    return ffi.string(buf)
end

print(U(L"Hello"))
fwrite(io.stdout, L"str: %s\n", L"✅ ハロー")

local fh = fopen(L"✅utf8dir/⭕utf8file.txt", L"r")
fwrite(io.stdout, L"%d\n", fh)
print(fread(fh, 256))
