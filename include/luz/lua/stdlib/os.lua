--- Operating System library ---

ffi.cdef[[
long os_execute(const char *cmd);
void os_sleep(unsigned long msec);
unsigned long os_gettime();
bool os_setenv(const char *name, const char *val);
const char *os_getenv(const char *name);
bool os_setcwd(const char *dir);
const char *os_getcwd(char *dest, size_t size);
]]

-- Execute OS command
function os.execute(cmd)
    debug.checkarg(cmd, "string")
    return ffi.C.os_execute(cmd)
end

-- Sleep process [milli seconds]
function os.sleep(msec)
    debug.checkarg(msec, "number")
    return ffi.C.os_sleep(msec)
end

-- Get system time [milli seconds]
function os.systime()
    return ffi.C.os_gettime()
end

-- Set system environmental variable
function os.setenv(name, val)
    debug.checkarg(name, "string", val, "string")
    return ffi.C.os_setenv(name, val)
end

-- Get system environemtal variable
function os.getenv(name)
    debug.checkarg(name, "string")
    return ffi.string(ffi.C.os_getenv(name))
end

-- Set current working directory
function os.setcwd(dir)
    debug.checkarg(dir, "string")
    return ffi.C.os_setcwd(dir)
end

-- Get current working directory
function os.getcwd()
    local dest = ffi.new("char[1025]", {})
    return ffi.string(ffi.C.os_getcwd(dest, 1024))
end
