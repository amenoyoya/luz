local function compile(infile, outfile, chunkname)
    print("compile: "..infile)
    
    local fh, msg = io.open(infile, "r")
    if not (fh) then
        error(msg)
    end

    local func = assert(load(fh:read"*a", chunkname))
    local dump, data = string.dump(func), ""
    for i = 1, dump:len() do
        data = data..dump:byte(i, i)..","
    end
    fh:close()
    fh, msg = io.open(outfile, "w")
    if not (fh) then
        error(msg)
    end
    fh:write(data)
    fh:close()
end

compile("table.lua", "table.cpp", "@stdlib://table")
compile("string.lua", "string.cpp", "@stdlib://string")
compile("os.lua", "os.cpp", "@stdlib://os")
compile("filesystem.lua", "filesystem.cpp", "@stdlib://filesystem")
compile("lpeg.lua", "lpeg.cpp", "@stdlib://lpeg")

-- ライブラリのロード順
--[[
    1. table
    2. string
    3. os
    4. filesystem
    5. lpeg
]]