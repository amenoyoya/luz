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
function debug.checkarg(...)
    local funcname = debug.getinfo(2, "n").name
    local args = {...}
    for i = 1, #args - 1, 2 do
        local t = type(args[i])
        if not typesame(t, args[i + 1]) then
            errorf(
                "function argument type error: '%s' argument %d expected %s, but got %s",
                funcname,
                math.floor((i - 1) / 2) + 1,
                args[i + 1],
                t
            )
        end
    end
end

-- Test
local test = {
    hello = function (str)
        debug.checkarg(str, "string")
        print(str)
    end,
    add = function (a, b)
        debug.checkarg(a, "string|number", b, "string|number")
        return type(a) == "number" and a + tonumber(b) or a .. tostring(b)
    end,
    print = function (str)
        debug.checkarg(str, "any")
        print(str)
    end,
}

local _, err = pcall(function()
    test.hello"Check start !"
    test.hello(nil)
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
