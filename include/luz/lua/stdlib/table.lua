--- Table library ---

-- reverse ipairs
function rpairs(t)
    local it = function(t,i)
        i = i - 1
        local v = t[i]
        if v == nil then return v end
        return i, v
    end
    return it, t, #t + 1
end

-- Copy table to table
-- @param copytable: copy recursively if `true` (default: false)
-- @param overwrite: (destKey)->(destAnotherKey, srcAnotherKey) (default: null)
--        if dest[key] exists when processing src[key]:
--           - designate `destAnotherKey` for copying destination
--           - if no `destAnotherKey` desiganted, skip copying
--           - store dest[key] into dest[srcAnotherKey] if `srcAnotherKey` is designated
function table.copy(src, dest, copytable, overwrite)
    local write = (type(overwrite) == 'function')
    
    if type(src) == 'table' and type(dest) == 'table' then
        for k, v in pairs(src) do
            if dest[k] ~= nil and write then -- if dest[key] exists
                local key, esckey = overwrite(k)
                -- store dest[key]
                if esckey then dest[esckey] = dest[k] end
                -- copy value
                if key then
                    if type(v) == 'table' and copytable then
                        local meta = getmetatable(v)
                        
                        dest[key] = {}
                        table.copy(v, dest[key], copytable, overwrite)
                        if meta then
                            setmetatable(dest[key], meta)
                        end
                    else
                        dest[key] = v
                    end
                end
            else -- simple copy
                if type(v) == 'table' and copytable then
                    dest[k] = {}
                    table.copy(v, dest[k], copytable, overwrite)
                else
                    dest[k] = v
                end
            end
        end
    end
end

-- Create the table's clone
function table:getclone()
    if type(self) == 'table' then
        local new = {}
        table.copy(self, new, true)
        return new
    else
        return self
    end
end

-- Get the table's slice
function table.slice(self, from, to)
    local new = {}
    from = from or 1
    to = to or #self
    for i = 1, to - from + 1 do
        new[i] = self[i + from - 1]
    end
    return new
end

-- Find element value in the table
-- @param {any} val: Search target value
--                   If you designate function, return value matching val(v: any) == true
-- @returns {any|nil} Matched index
function table.find(self, val)
    local f = type(val) == "function" and val or function(v) return v == val end
    for k, v in ipairs(self) do
      if f(v) then return k end
    end
    return nil
end


--- Serialize ---
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
    return "\\" .. (escape_char_map[c] or string.format("u%04x", c:u8byte()))
end

-- @private Process of escaping
local function escape_string(str)
    return str:u8gsub('[%z\1-\31\\"]', escape_char)
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


--- Class definition function ---

--[[
-- Define new class `App`
App = class {
    constructor = function(self, id)
        self.id = 0
        self.map = {}
        self:setID(id)
    end,
    destructor = function(self)
        self.map = {}
        print "See you..."
    end,
    operator = {
        __newindex = function(self, index, val) self.map[index] = val end,
        __index = function(self, index) return self.map[index]  end,
    },
    getID = function(self) return self.id end,
    setID = function(self, id) self.id = id or 0 end,
}

local app = App.new(100)
app[1] = "Hello"
print(app:getID()) --> 100
print(app[1]) --> "Hello"
app = nil
collectgarbage "collect" --> "See you..."

-- Define `MyApp` extended from `App`
MyApp = class(App) {
    constructor = function(self, ...)
        -- call parent class constructor
        App.constructor(self, ...)
    end,
    
    setID = function(self, id)
        -- call parent method
        App.setID(self, (id or 0) * 5)
    end
}

local myapp = MyApp.new(100)
print(myapp:getID()) --> 500
--]]

local function createclass(define)
    define.new = function (...)
        local obj = table.getclone(define) -- new instance
        -- destructor
        if type(obj.destructor) == 'function' then
            obj.__udata__ = newproxy(true)
            getmetatable(obj.__udata__).__gc = function()
                obj:destructor()
            end
        end
        -- execute constructor
        if type(obj.constructor) == 'function' then
            obj:constructor(...)
        end
        -- metatable operators
        if type(obj.operator) == 'table' then
            setmetatable(obj, obj.operator)
            obj.operator = nil
        end
        return obj
    end
    return define
end

function class(define)
    local extend = (type(define.__extend) == 'function') -- is extended ?
    
    define.__extend = function(child)
        table.copy(define, child, false, function(name) end) -- 子クラスに同名のメンバがある場合は，親メンバのコピーは行わない
        return createclass(child)
    end
    -- class definition
    if extend then
        return define.__extend -- extended class
    else
        return createclass(define) -- basic class
    end
end

--[[
    operators
    +:__add, -:__sub, *:__mul, /:__div, %:__mod, ^:__pow,
    ..(concat operator):__concat, -(minus symbol):__unm,
    []: __index, []=: __newindex
    #(length operator):__len, ==:__eq, <:__lt, <=:__le
]]


--- Copy namespace into global table ---
-- @param overwrite: (destKey)->(destAnotherKey, srcAnotherKey) (default: null)
--        if dest[key] exists when processing src[key]:
--           - designate `destAnotherKey` for copying destination
--           - if no `destAnotherKey` desiganted, skip copying
--           - store dest[key] into dest[srcAnotherKey] if `srcAnotherKey` is designated
function using(namespace, overwrite)
    table.copy(namespace, _G, false, overwrite)
end
