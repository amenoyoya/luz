%~d1
cd "%~dp1"

call "%~dp0vcvars.bat"

%compile% "%~n1.cpp"
link.exe /ERRORREPORT:NONE /NOLOGO /MACHINE:X86 /OUT:"%~n1.exe" "%~n1.obj"
del "%~n1.obj"
del "%~n1.lib"
del "%~n1.exp"
