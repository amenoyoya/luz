cd $(dirname $0)

export CPLUS_INCLUDE_PATH="$(pwd)/../include:$(pwd)/../extlib/LuaJIT-2.1.0-beta3/src:$(pwd)/../extlib/zlib-1.2.8:$(pwd)/../extlib/sol2-3.2.2"

g++ -c -O2 -s -DNDEBUG ./luz/*.cpp
ar rcs libluz.a ./*.o
rm ./*.o
mv libluz.a ../dist/lib/x64/
