-- compile lua => sym, and append sym to resource
-- @param {fs.zip.archiver} arc
-- @param {string} luafile
-- @param {string} rootdir
function append_lua(arc, luafile, rootdir)
    local resname = luafile:sub(rootdir:len() + 2):sub(1, -5)
    local f, err = load(fs.readfile(luafile), "@luz://" .. resname)
    if f == nil then
        error(err)
    end
    
    local bytecode = string.dump(f)
    if not arc:append(bytecode, bytecode:len(), resname .. ".sym") then
        errorf("failed to append %s", resname .. ".sym")
    end
    printf("%s.lua has been bundled into luz://%s.sym\n", resname, resname)
end

-- open luz application as archive
local luz = package.__dir .. "/luz" .. (ffi.os == "Windows" and ".exe" or "")
local arc = fs.zip.open(luz, "w+")

if arc == nil then
    errorf("failed to open '%s' as zip archive", luz)
end

local dir = package.__dir .. "/resource"
local files = fs.enumfiles(dir)

for _, file in ipairs(files) do
    if fs.path.ext(file.path) == ".lua" then
        local filepath, _ = file.path:gsub("\\", "/")
        append_lua(arc, filepath, dir)
    end
end

arc:close()
