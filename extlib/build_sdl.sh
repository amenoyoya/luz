cd "$(dirname $0)/SDL2-2.0.16"

./configure --disable-shared --enable-static
make

mv ./build/.libs/libSDL2.a ../../dist/lib/x64/
mv ./build/.libs/libSDL2main.a ../../dist/lib/x64/

make clean
