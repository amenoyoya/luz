%~d0
cd %~dp0

call ..\vcvars.bat

%compile% luz\*.cpp
lib.exe /OUT:"libluz.lib" /NOLOGO *.obj
move libluz.lib ..\dist\lib\x86\
del *.obj
