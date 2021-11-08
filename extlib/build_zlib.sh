cd "$(dirname $0)/zlib-1.2.8"

cp -f zconf.h.in zconf.h

gcc -c -O2 -s -DNDEBUG *.c
ar rcs libzlib.a *.o

mv libzlib.a ../../dist/lib/x64/libzlib.a
rm *.o
