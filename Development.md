# Development

- Linker must **dynamic link** all libraries to the application
    - For using luajit FFI (all libraries must be callable from `dlsym`)

## Development in Windows 10

### Setup
```powershell
# install visual c++ 2019 build tools by chocolatey
> choco install -y visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Workload.VCTools --includeRecommended --includeOptional --passive
```

Make sure `C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat` has been installed by Chocolatey.

Or you must edit `./vcvars.bat` file following to your Visual C++ environment.

### Build external libraries
```powershell
# Build LuaJIT
> .\extlib\build_luajit.bat

# Build zlib
> .\extlib\build_zlib.bat
```

***

## Development in Ubuntu 20.04

### Setup
```bash
# install gcc etc
$ sudo apt install -y build-essential

# install 32bit development libraries
# $ sudo apt install -y libc6-dev-i386
```

### Build external libraries
```bash
# Build LuaJIT
$ /bin/bash ./extlib/build_luajit.sh

# Build zlib
$ /bin/bash ./extlib/build_zlib.sh
```
