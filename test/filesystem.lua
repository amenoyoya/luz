assert(os.setcwd(package.__dir))
print("current working directory:", os.getcwd())
print("current script file:", package.__file, fs.path.stat(package.__file))
print("invalid file state:", fs.path.stat"invalid?file!")

assert(fs.copyfile(package.__file, "✅copied/test.lua"))
assert(fs.copydir("✅copied", "❗party"))
assert(fs.rename("✅copied", "❗party/✨subdir"))

print("file list in ❗party/", fs.enumfiles"❗party")
assert(fs.rmdir"❗party")

-- wait for removal of directory
while fs.path.isdir"❗party" do
    os.sleep(10)
end

assert(#fs.enumfiles"❗party" == 0)

print("this file:", fs.readfile(package.__file))
print("write bytes:", fs.writefile("⭐", "⭐🌍🌛"))

local file = fs.open("⭐")
assert(file:readline() == "⭐🌍🌛")
file:close()

assert(fs.rmfile"⭐")

--- zip, unz ---
-- assert(fs.zip.compress("../src/", "./source.zip", 9, "password"))

-- @private Compress the directory (base)
local function compress(zip, dirPath, baseDirLen, password, rootDir)
    local f = fs.opendir(dirPath)
    
    if f == nil then return false end
    repeat
        if f:readname() ~= "." and f:readname() ~= ".." then
            local path = f:readpath()
            if fs.path.isdir(path) then
                -- process recursively
                if not compress(zip, path, baseDirLen, password, rootDir) then return false end
            elseif fs.path.isfile(path) then
                -- append file into zip
                if not zip:append_file(path, rootDir .. path:sub(baseDirLen + 1), password) then return false end
            end
        end
    until not f:seek()
    f:close()
    return true
end

-- Compress the directory to zip
-- @param {string} dir: source directory path
-- @param {string} output: output zip file path
-- @param {number} level: compress level (default: 0)
-- @param {string|nil} password: zip password (default: nil)
-- @param {string} mode: "w"(default)|"w+"|"a"
-- @param {string} root: root of local file path in the zip (default: "")
-- @returns {boolean}
function zip_compress(dir, output, level, password, mode, root)
    if not fs.path.isdir(dir) then return false end

    local zip = fs.zip.open(output, mode, level)
    if zip == nil then return false end

    local result = compress(zip, dir, fs.path.append_slash(dir):len(), password, root and fs.path.append_slash(root) or "")
    zip:close()
    return result
end

-- Uncompress the zip into directory
-- @param {string} zip: source zip file
-- @param {string} dir: output directory path
-- @param {string|nil} password (default: nil)
-- @returns {boolean}
function unz_uncompress(zip, dir, password)
    local unz = fs.unz.open(zip)
    if unz == nil then return false end
    repeat
        local info = unz:info(true, password)
        if info == nil then error"failed to get information of local file" end
        
        local file = fs.open(fs.path.append_slash(dir) .. info.filename, "wb")
        if file == nil then error"failed to open extracted file" end
        file:write(info.content, info.uncompressed_size)
        file:close()
    until not unz:locate_next()
    unz:close()
    return true
end

print"compress: ../src/ => ./❓source.zip"
assert(zip_compress("../src/", "./❓source.zip", 9, "password"))

print"uncompress: ./❓source.zip => ./❗extracted/src/"
assert(unz_uncompress("./❓source.zip", "./❗extracted/src/", "password"))

local dir = "./❗extracted/src/"
for _, file in ipairs(fs.enumfiles(dir)) do
    assert(fs.readfile(file.path) == fs.readfile("../src/" .. file.path:sub(dir:len() + 1)))
    print("✅", file.path)
end

assert(fs.rmfile"❓source.zip")
assert(fs.rmdir"./❗extracted")
