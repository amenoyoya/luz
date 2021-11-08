call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

@set include=%~dp0include;%include%
@set include=%~dp0extlib\LuaJIT-2.1.0-beta3\src;%~dp0extlib\tolua++-1.0.93\include;%~dp0extlib\sol2-3.2.2;%~dp0extlib\zlib-1.2.8;%include%
@set include=%~dp0extlib\SDL2-2.0.16\include;%include%
@set include=%~dp0extlib\SDL2_image-2.0.5;%~dp0extlib\SDL2_image-2.0.5\external\jpeg-9b;%~dp0extlib\SDL2_image-2.0.5\external\libpng-1.6.37;%~dp0extlib\SDL2_image-2.0.5\external\libwebp-1.0.2;%~dp0extlib\SDL2_image-2.0.5\external\tiff-4.0.9\libtiff;%include%
@set include=%~dp0extlib\SDL2_ttf-2.0.15;%~dp0extlib\SDL2_ttf-2.0.15\external\freetype-2.9.1\include;%include%
@set lib=%~dp0dist\lib\x86;%lib%

:: dynamic link /MD
@set compile=cl.exe /c /nologo /std:c++17 /W3 /WX- /O2 /Oi /Oy- /D _CRT_SECURE_NO_WARNINGS /D WIN32 /D NDEBUG /D _MBCS /Gm- /EHsc /MD /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Gd /analyze- /errorReport:none
