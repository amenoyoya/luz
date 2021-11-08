cd "$(dirname $0)/LuaJIT-2.1.0-beta3"

# enable lua52compat
sed -i 's/^# *XCFLAGS *+= *-DLUAJIT_ENABLE_LUA52COMPAT/XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT/' src/Makefile

make
mv src/libluajit.a ../../dist/lib/x64/libluajit.a
mv src/luajit ../../dist/bin/x64/luajit
make clean
