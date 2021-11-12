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
local function checkarg(n, ...)
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

-- Test
local test = {
    hello = function (str)
        checkarg(1, str, "string")
        print(str)
    end,
    fmt = function (stdout, format, ...)
        checkarg(2, stdout, "cdata|userdata", format, "string")
        ffi.C.fputws(format:format(...):u8towcs(), stdout)
    end,
    add = function (a, b)
        checkarg(2, a, "string|number", b, "string|number")
        return type(a) == "number" and a + tonumber(b) or a .. tostring(b)
    end,
    print = function (str)
        checkarg(1, str, "any")
        print(str)
    end,
}

local _, err = pcall(function()
    test.hello"Check start !"
    test.hello(nil)
end)

if err then eprint(err) end

_, err = pcall(function()
    test.fmt(io.stdout, "%d\n", 10)
    test.fmt(io.stderr, nil, 10)
end)

if err then eprint(err) end

_, err = pcall(function()
    print(test.add(10, 20))
    print(test.add("I'm ", 123))
    print(test.add(true, false))
end)

if err then eprint(err) end

_, err = pcall(function()
    test.print(10, 20)
    test.print("I'm ", 123)
    test.print(true, false)
end)

if err then eprint(err) end
