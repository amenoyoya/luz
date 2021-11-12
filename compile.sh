dir=$(cd $(dirname $0); pwd)

export CPLUS_INCLUDE_PATH="$dir/include:$dir/extlib/LuaJIT-2.1.0-beta3/src:$dir/extlib/zlib-1.2.8:$dir/extlib/sol2-3.2.2"
export CPLUS_INCLUDE_PATH="$dir/extlib/SDL2-2.0.16/include:$CPLUS_INCLUDE_PATH"

# export each functions to be callable by dlsym: -rdynamic
## => you must not designate -static option
g++ -O2 -std=c++17 -L"$dir/dist/lib/x64" -static-libstdc++ -o ${1%.*} $1 -lluajit -lzlib -ldl -lpthread -rdynamic
