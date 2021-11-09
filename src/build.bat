%~d0
cd %~dp0

call ..\vcvars.bat

:: generate lua stdlib
call ..\include\luz\lua\_generate_stdlib_code.bat

cd %~dp0
%compile% miniluz.cpp
link.exe /ERRORREPORT:NONE /NOLOGO /MACHINE:X86 /OUT:miniluz.exe miniluz.obj
del *.obj
del *.lib
del *.exp
move miniluz.exe ..\dist\bin\x86\
