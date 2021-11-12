-- version
luz = {
    version = "v0.1.0"
}

-- help text
local helptext = [==[
Luz is a LuaJIT script engine.

Usage:
    luz [command] [script_file] [arguments]
    
The commands are:
    help        display information about Luz script engine
    version     display Luz version
    transpile   transpile Teal code to Lua code
            Usage:          $ luz transpile <input_teal_script_file> <output_lua_script_file>
            Description:    Luz will transpile <input_teal_script_file> to <output_lua_script_file>
    compile     compile Lua / Teal code to byte-code
            Usage:          $ luz compile <input_lua_script_file> <output_byte_code_file>
            Description:    Luz will compile <input_lua_script_file> to <output_byte_code_file>
    test        execute test codes
            Usage:          $ luz test [directory (default: ./)]
            Description:    Luz will execute test script files like "*_test.lua", "*_test.tl" in the <directory> and the sub directories

The script file will be executed:
    If the "main.lua" file exists at the directory containing Luz: execute a plain Lua script file
    Or if the "main.sym" file exists at the directory containing Luz: execute a compiled Lua byte-code file
    Or if the "main.tl" file exists at the directory containing Luz: execute a Teal script file
    Or if the command line arguments[1] is "*.lua" file: execute a plain Lua script file
    Or if the command line arguments[1] is "*.sym" file: execute a ompiled Lua byte-code file
    Or if the command line arguments[1] is "*.tl" file: execute a Teal script file

Luz will be executed as interactive-mode if there are no commands and script files.
]==]

-- print version info
local function printver()
    printf("Luz Engine %s -- Copyright (C) 2021 amenoyoya. https://github.com/amenoyoya/luz\n", luz.version)
    printf("%s -- Copyright (C) 2005-2017 Mike Pall. http://luajit.org/\n", jit.version)
end

-- os.argv[0] <= The Luz engine path
os.argv[0] = os.argv[1]
table.remove(os.argv, 1)

-- package path for searching
package.path = "?.lua;?.sym;?.tl;?/init.lua;?/init.sym;?/init.tl;" .. package.path
package.cpath = "?.dll;?.so;" .. package.cpath

-- extended package loader: search from current application (os.arv[0]) resource
local __resource = fs.unz.open(os.argv[0])

if __resource == nil then
    return "\n\tLuz has no resource: '" .. os.argv[0] .. "'"
end

table.insert(package.loaders, 1, function (module_name)
    local error_message = ""
    module_name = module_name:gsub("%.", "/") -- "." => "/"

    for entry in package.path:gmatch"[^;]+" do
        local modname = entry:replace("?", module_name)
        if __resource:locate_name(modname) then
            local info = __resource:info(true)

            if info.content:len() > 0 then
                local loader, err = load(info.content, "@luz://" .. module_name)
                if loader == nil then error(err) end
                return loader
            end
        end
        error_message = error_message .. "\n\tno file '" .. modname .. "' in '" .. os.argv[0] .. "' resource"
    end
    return error_message
end)

-- require luz://@system
-- * teal language system
-- * relative require package system
require "@system"

-- CLI commands
local commands = {
    help = function ()
        print(helptext)
    end,
    
    version = function ()
        printver()
    end,

    transpile = function ()
        local input, output = os.argv[2], os.argv[3]

        if input == nil or not fs.path.isfile(input) then
            error("no Teal script file is input")
        end
        if output == nil then
            error("no output Lua script file path is specified")
        end

        local code, err = teal.transpile(fs.readfile(input), input)
        if code == nil then
            error(err)
        end
        if 0 == fs.writefile(output, code) then
            errorf("cannot write Lua script into '%s'", output)
        end
    end,

    compile = function ()
        local input, output = os.argv[2], os.argv[3]

        if input == nil or not fs.path.isfile(input) then
            error("no Lua script file is input")
        end
        if output == nil then
            error("no output Lua byte-code file path is specified")
        end

        local f, err = loadfile(input)
        if f == nil then error(err) end

        local code = string.dump(f)
        if 0 == fs.writefile(output, code) then
            errorf("cannot write Lua byte-code into '%s'", output)
        end
    end,

    test = function ()
        -- Execute test codes in the target directory or current directory, and measure execution time
        -- * Test codes: `*_test.lua` or `*_test.tl`
        local files = fs.enumfiles(os.argv[2] or ".", -1, "file")
        local ok, ng = 0, 0 -- count of test results
        local teststart = os.systime() -- measure execution time

        for _, file in ipairs(files) do
            if file.path:match("_test%.lua$") or file.path:match("_test%.tl$") then
                -- test code execution time: lua or teal
                local start = os.systime()
                local f, err = loadfile(file.path)

                if f then
                    local result, err = pcall(f, "@" .. file.path)

                    if result then -- OK
                        printf("✅ %s (%d ms)\n", file.path, os.systime() - start)
                        ok = ok + 1
                    else -- NG
                        printf("❌ %s (%d ms)\n", file.path, os.systime() - start)
                        print(err)
                        ng = ng + 1
                    end
                else -- syntax error
                    printf("❌ %s (%d ms)\n", file.path, os.systime() - start)
                    print(err)
                    ng = ng + 1
                end
            end
        end
        -- display test summary
        printf("\nTests:\t%d failed, %d total\n", ng, ok + ng)
        printf("Time:\t%d ms\n\n", os.systime() - teststart)
    end,
}

-- case: argv[1] is CLI commands
local f = commands[os.argv[1]]

if f then
    f()
    os.exit(0)
end

-- case: "{__dir}/main.lua" or "{__dir}/main.sym" or "{__dir}/main.tl" script file exists
local dir = fs.path.append_slash(fs.path.parentdir(os.argv[0]))
local function do_main_script(scriptfile)
    if fs.path.isfile(scriptfile) then
        table.insert(os.argv, 1, scriptfile) -- os.argv[1] <= main script file
        dofile(scriptfile)
        os.exit(0)
    end
end

do_main_script(dir .. "main.lua")
do_main_script(dir .. "main.sym")
do_main_script(dir .. "main.tl")

-- case: argv[1] is "*.lua" or "*.sym" or "*.tl" script file
if os.argv[1] then
    local scriptfile = fs.path.complete(os.argv[1])
    local ext = fs.path.ext(scriptfile)
    if (ext == ".lua" or ext == ".sym" or ext == ".tl") and fs.path.isfile(scriptfile) then
        dofile(scriptfile)
        os.exit(0)
    end
end

-- case: no main script file exists
--- => execute interactive-mode
printver()
debug.debug(os.argv[0])
