# luz

A LuaJIT script engine for a standalone utility application.

## TODO

- Luz is unstable because of Lua's memory management system
    - Currently Lua's garbage collection and C pointers don't go well

***
## Features

- Supports relative module searching system.
    - If you write `require` function with module name including `/`, the module will be searched from relative file path.
        - In this case, `.` is not replaced into `/`. So you want to require `./relative/module.lua`, you must write like below.
            - ✅ `require("./relative/module")`
            - ❌ `require("./relative.module")`
    - If module name has no `/`, the default require system will be executed.
        - e.g. `require("compat53.module")`
            - Search `compat53/module.lua`, `compat53/module/init.lua` ...etc
- Supports [Teal](https://github.com/teal-language/tl) - a typed dialect of Lua.
    - `teal.transpile(code: string, code_name: string): {luacode: string, err: string}`
        - Transpile Teal code to Lua code.
    - `teal.*`: See https://github.com/teal-language/tl/blob/master/docs/tutorial.md
- Supports [lua-compat-53](https://github.com/keplerproject/lua-compat-5.3)
    - `require("compat53")`
        - Load `compa53` module from the Aula engine resource (appended zip file). 
    - `require("compat53.module")`
        - Load `compa53.module` module from the Aula engine resource (appended zip file).

***

## Environment

- OS:
    - Windows 10
        - Shell: PowerShell
    - Ubuntu 20.04
        - Shell: bash
- Build tools:
    - Microsoft Visual C++ 2019 Community Edition
    - gcc: `9.3.0`
- Editor:
    - VSCode

***

## Development

Read [Development.md](./Development.md)
