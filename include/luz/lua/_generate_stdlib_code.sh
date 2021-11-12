cd "$(dirname $0)"

export PATH="$(pwd)/../../../dist/bin/x64:$PATH"

cd ./stdlib
luajit _compile.lua
