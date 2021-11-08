%~d0
cd %~dp0

call ..\vcvars.bat

cd zlib-1.2.8
copy zconf.h.in zconf.h

%compile% /wd4996 *.c
lib.exe /OUT:"zlib.lib" /NOLOGO *.obj

del *.obj
move zlib.lib ..\..\dist\lib\x86\
