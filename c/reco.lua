ffi.cdef[[
    int MessageBoxW(unsigned long hwnd, const wchar_t *text, const wchar_t *caption, unsigned int type);
]]

-- ffi.C.MessageBoxW(0, ("Hello"):u8towcs(), ("title"):u8towcs(), 0) 

function L(str)
    local wstr = reco.new(str:len() * ffi.sizeof("wchar_t"))
    if wstr == nil then return nil end
    ffi.C.u8towcs(wstr:cast"wchar_t*", str, wstr.size)
    return wstr:cast"wchar_t*"
end

ffi.C.MessageBoxW(0, L"hello", L"title", 0)
