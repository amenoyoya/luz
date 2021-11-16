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

---

local function myreco_new(name)
    local myname = name
    local obj = reco.record.new(
        function ()
            return tonumber(ffi.cast("unsigned long", ffi.cast("const char*", myname))), myname:len()
        end,
        function (handler)
            printf("see you %s\n", ffi.string(ffi.cast("const char*", handler)))
        end
    )
    return obj:addr() ~= 0 and obj or nil
end

local iam = myreco_new"test"
printf("I'm %s\n", iam:tostr())

iam = myreco_new"✅"
iam:close()
printf("I'm %s\n", iam:tostr())
iam:close()
