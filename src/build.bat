%~d0
cd %~dp0

call ..\vcvars.bat

:: generate lua stdlib
call ..\include\luz\lua\_generate_stdlib_code.bat

cd %~dp0
%compile% miniluz.cpp
link.exe /ERRORREPORT:NONE /NOLOGO /MACHINE:X86 /OUT:miniluz.exe miniluz.obj

%compile% luz.cpp
link.exe /ERRORREPORT:NONE /NOLOGO /MACHINE:X86 /OUT:luz.exe luz.obj

:: bundle lua resources
call miniluz.exe bundle.lua

del *.obj
del *.lib
del *.exp
@REM move miniluz.exe ..\dist\bin\x86\
@REM move luz.exe ..\dist\bin\x86\
