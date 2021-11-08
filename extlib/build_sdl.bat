%~d0
cd %~dp0

call ..\vcvars.bat

:: SDL2 ::
cd SDL2-2.0.16
set cmpl=%compile% /D"_UNICODE" /D"UNICODE" /D"HAVE_LIBC" /D"NDEBUG"
%cmpl% src\atomic\SDL_atomic.c
%cmpl% src\atomic\SDL_spinlock.c
%cmpl% src\audio\directsound\SDL_directsound.c
%cmpl% src\audio\disk\SDL_diskaudio.c
%cmpl% src\audio\dummy\SDL_dummyaudio.c
%cmpl% src\audio\SDL_audio.c
%cmpl% src\audio\SDL_audiocvt.c
%cmpl% src\audio\SDL_audiodev.c
%cmpl% src\audio\SDL_audiotypecvt.c
%cmpl% src\audio\SDL_mixer.c
%cmpl% src\audio\SDL_wave.c
%cmpl% src\audio\winmm\SDL_winmm.c
%cmpl% src\audio\wasapi\SDL_wasapi.c
%cmpl% src\audio\wasapi\SDL_wasapi_win32.c
%cmpl% src\core\windows\SDL_hid.c
%cmpl% src\core\windows\SDL_windows.c
%cmpl% src\core\windows\SDL_xinput.c
%cmpl% src\cpuinfo\SDL_cpuinfo.c
%cmpl% src\dynapi\SDL_dynapi.c
%cmpl% src\events\SDL_clipboardevents.c
%cmpl% src\events\SDL_displayevents.c
%cmpl% src\events\SDL_dropevents.c
%cmpl% src\events\SDL_events.c
%cmpl% src\events\SDL_gesture.c
%cmpl% src\events\SDL_keyboard.c
%cmpl% src\events\SDL_mouse.c
%cmpl% src\events\SDL_quit.c
%cmpl% src\events\SDL_touch.c
%cmpl% src\events\SDL_windowevents.c
%cmpl% src\file\SDL_rwops.c
%cmpl% src\filesystem\windows\SDL_sysfilesystem.c
%cmpl% src\haptic\dummy\SDL_syshaptic.c
%cmpl% src\haptic\SDL_haptic.c
%cmpl% src\haptic\windows\SDL_dinputhaptic.c
%cmpl% src\haptic\windows\SDL_windowshaptic.c
%cmpl% src\haptic\windows\SDL_xinputhaptic.c
%cmpl% src\hidapi\SDL_hidapi.c
%cmpl% src\joystick\dummy\SDL_sysjoystick.c
%cmpl% src\joystick\hidapi\SDL_hidapijoystick.c
%cmpl% src\joystick\hidapi\SDL_hidapi_gamecube.c
%cmpl% src\joystick\hidapi\SDL_hidapi_luna.c
%cmpl% src\joystick\hidapi\SDL_hidapi_ps4.c
%cmpl% src\joystick\hidapi\SDL_hidapi_ps5.c
%cmpl% src\joystick\hidapi\SDL_hidapi_rumble.c
%cmpl% src\joystick\hidapi\SDL_hidapi_stadia.c
%cmpl% src\joystick\hidapi\SDL_hidapi_switch.c
%cmpl% src\joystick\hidapi\SDL_hidapi_xbox360.c
%cmpl% src\joystick\hidapi\SDL_hidapi_xbox360w.c
%cmpl% src\joystick\hidapi\SDL_hidapi_xboxone.c
%cmpl% src\joystick\SDL_gamecontroller.c
%cmpl% src\joystick\SDL_joystick.c
%cmpl% src\joystick\virtual\SDL_virtualjoystick.c
%cmpl% src\joystick\windows\SDL_dinputjoystick.c
%cmpl% src\joystick\windows\SDL_mmjoystick.c
%cmpl% src\joystick\windows\SDL_rawinputjoystick.c
%cmpl% src\joystick\windows\SDL_windowsjoystick.c
%cmpl% src\joystick\windows\SDL_windows_gaming_input.c
%cmpl% src\joystick\windows\SDL_xinputjoystick.c
%cmpl% src\libm\e_atan2.c
%cmpl% src\libm\e_exp.c
%cmpl% src\libm\e_fmod.c
%cmpl% src\libm\e_log.c
%cmpl% src\libm\e_log10.c
%cmpl% src\libm\e_pow.c
%cmpl% src\libm\e_rem_pio2.c
%cmpl% src\libm\e_sqrt.c
%cmpl% src\libm\k_cos.c
%cmpl% src\libm\k_rem_pio2.c
%cmpl% src\libm\k_sin.c
%cmpl% src\libm\k_tan.c
%cmpl% src\libm\s_atan.c
%cmpl% src\libm\s_copysign.c
%cmpl% src\libm\s_cos.c
%cmpl% src\libm\s_fabs.c
%cmpl% src\libm\s_floor.c
%cmpl% src\libm\s_scalbn.c
%cmpl% src\libm\s_sin.c
%cmpl% src\libm\s_tan.c
%cmpl% src\loadso\windows\SDL_sysloadso.c
%cmpl% src\locale\SDL_locale.c
%cmpl% src\locale\windows\SDL_syslocale.c
%cmpl% src\misc\SDL_url.c
%cmpl% src\misc\windows\SDL_sysurl.c
%cmpl% src\power\SDL_power.c
%cmpl% src\power\windows\SDL_syspower.c
%cmpl% src\render\direct3d11\SDL_shaders_d3d11.c
%cmpl% src\render\direct3d\SDL_render_d3d.c
%cmpl% src\render\direct3d11\SDL_render_d3d11.c
%cmpl% src\render\direct3d\SDL_shaders_d3d.c
%cmpl% src\render\opengl\SDL_render_gl.c
%cmpl% src\render\opengl\SDL_shaders_gl.c
%cmpl% src\render\opengles2\SDL_render_gles2.c
%cmpl% src\render\opengles2\SDL_shaders_gles2.c
%cmpl% src\render\SDL_d3dmath.c
%cmpl% src\render\SDL_render.c
%cmpl% src\render\SDL_yuv_sw.c
%cmpl% src\render\software\SDL_blendfillrect.c
%cmpl% src\render\software\SDL_blendline.c
%cmpl% src\render\software\SDL_blendpoint.c
%cmpl% src\render\software\SDL_drawline.c
%cmpl% src\render\software\SDL_drawpoint.c
%cmpl% src\render\software\SDL_render_sw.c
%cmpl% src\render\software\SDL_rotate.c
%cmpl% src\SDL.c
%cmpl% src\SDL_assert.c
%cmpl% src\SDL_dataqueue.c
%cmpl% src\SDL_error.c
%cmpl% src\SDL_hints.c
%cmpl% src\SDL_log.c
%cmpl% src\sensor\dummy\SDL_dummysensor.c
%cmpl% src\sensor\SDL_sensor.c
%cmpl% src\sensor\windows\SDL_windowssensor.c
%cmpl% src\stdlib\SDL_crc32.c
%cmpl% src\stdlib\SDL_getenv.c
%cmpl% src\stdlib\SDL_iconv.c
%cmpl% src\stdlib\SDL_malloc.c
%cmpl% src\stdlib\SDL_qsort.c
%cmpl% src\stdlib\SDL_stdlib.c
%cmpl% src\stdlib\SDL_string.c
%cmpl% src\stdlib\SDL_strtokr.c
%cmpl% src\thread\generic\SDL_syscond.c
%cmpl% src\thread\SDL_thread.c
%cmpl% src\thread\windows\SDL_syscond_srw.c
%cmpl% src\thread\windows\SDL_sysmutex.c
%cmpl% src\thread\windows\SDL_syssem.c
%cmpl% src\thread\windows\SDL_systhread.c
%cmpl% src\thread\windows\SDL_systls.c
%cmpl% src\timer\SDL_timer.c
%cmpl% src\timer\windows\SDL_systimer.c
%cmpl% src\video\dummy\SDL_nullevents.c
%cmpl% src\video\dummy\SDL_nullframebuffer.c
%cmpl% src\video\dummy\SDL_nullvideo.c
%cmpl% src\video\SDL_blit.c
%cmpl% src\video\SDL_blit_0.c
%cmpl% src\video\SDL_blit_1.c
%cmpl% src\video\SDL_blit_A.c
%cmpl% src\video\SDL_blit_auto.c
%cmpl% src\video\SDL_blit_copy.c
%cmpl% src\video\SDL_blit_N.c
%cmpl% src\video\SDL_blit_slow.c
%cmpl% src\video\SDL_bmp.c
%cmpl% src\video\SDL_clipboard.c
%cmpl% src\video\SDL_egl.c
%cmpl% src\video\SDL_fillrect.c
%cmpl% src\video\SDL_pixels.c
%cmpl% src\video\SDL_rect.c
%cmpl% src\video\SDL_RLEaccel.c
%cmpl% src\video\SDL_shape.c
%cmpl% src\video\SDL_stretch.c
%cmpl% src\video\SDL_surface.c
%cmpl% src\video\SDL_video.c
%cmpl% src\video\SDL_vulkan_utils.c
%cmpl% src\video\SDL_yuv.c
%cmpl% src\video\windows\SDL_windowsclipboard.c
%cmpl% src\video\windows\SDL_windowsevents.c
%cmpl% src\video\windows\SDL_windowsframebuffer.c
%cmpl% src\video\windows\SDL_windowskeyboard.c
%cmpl% src\video\windows\SDL_windowsmessagebox.c
%cmpl% src\video\windows\SDL_windowsmodes.c
%cmpl% src\video\windows\SDL_windowsmouse.c
%cmpl% src\video\windows\SDL_windowsopengl.c
%cmpl% src\video\windows\SDL_windowsopengles.c
%cmpl% src\video\windows\SDL_windowsshape.c
%cmpl% src\video\windows\SDL_windowsvideo.c
%cmpl% src\video\windows\SDL_windowsvulkan.c
%cmpl% src\video\windows\SDL_windowswindow.c
%cmpl% src\video\yuv2rgb\yuv_rgb.c
lib.exe /OUT:"SDL2.lib" /NOLOGO *.obj
del *.obj

%cmpl% src\main\windows\SDL_windows_main.c
lib.exe /OUT:"SDL2main.lib" /NOLOGO *.obj
del *.obj
move SDL2.lib ..\..\dist\lib\x86\
move SDL2main.lib ..\..\dist\lib\x86\

:: SDL_image ::
cd %~dp0SDL2_image-2.0.5\external\jpeg-9b
copy jconfig.h jconfig.h.bak
copy /y jconfig.vc jconfig.h
%compile% jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c jccolor.c ^
        jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c ^
        jcomapi.c jcparam.c jcprepct.c jcsample.c jctrans.c jdapimin.c ^
        jdapistd.c jdarith.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c ^
        jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c ^
        jdmerge.c jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c ^
        jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c jquant1.c ^
        jquant2.c jutils.c jmemmgr.c
move /y jconfig.h.bak jconfig.h
lib.exe /OUT:"libjpeg.lib" /NOLOGO *.obj
move libjpeg.lib ..\..\..\..\dist\lib\x86\
del *.obj

cd ..\libpng-1.6.37
%compile% *.c
lib.exe /OUT:"libpng.lib" /NOLOGO *.obj
move libpng.lib ..\..\..\..\dist\lib\x86\
del *.obj

cd ..\libwebp-1.0.2
nmake -f Makefile.vc CFG=release-static OBJDIR=. RTLIBCFG=static target=all
move release-static\x86\lib\*.lib ..\..\..\..\dist\lib\x86\
rmdir /s /q release-static

cd ..\tiff-4.0.9
set JPEGDIR=%~dp0\SDL2_image-2.0.5\external\jpeg-9b
copy libtiff/tiffconf.h libtiff/tiffconf.h.bak
copy /y libtiff/tiffconf.vc.h libtiff/tiffconf.h
nmake -f makefile.vc
move /y libtiff/tiffconf.h.bak libtiff/tiffconf.h
move libtiff\*.lib ..\..\..\..\dist\lib\x86\
nmake -f makefile.vc clean

cd ..\..\
%compile% IMG*.c
lib.exe /OUT:"SDL2_image.lib" /NOLOGO *.obj
move SDL2_image.lib ..\..\dist\lib\x86\
del *.obj

:: SDL_ttf ::
cd %~dp0SDL2_ttf-2.0.15
set cmpl=%compile% /D"_UNICODE" /D"UNICODE" /D"NDEBUG" /D"_LIB" /D"FT2_BUILD_LIBRARY"
%cmpl% external\freetype-2.9.1\src\autofit\autofit.c
%cmpl% external\freetype-2.9.1\src\bdf\bdf.c
%cmpl% external\freetype-2.9.1\src\cff\cff.c
%cmpl% external\freetype-2.9.1\src\base\ftbase.c
%cmpl% external\freetype-2.9.1\src\base\ftbitmap.c
%cmpl% external\freetype-2.9.1\src\base\ftfstype.c
%cmpl% external\freetype-2.9.1\src\base\ftgasp.c
%cmpl% external\freetype-2.9.1\src\cache\ftcache.c
%cmpl% external\freetype-2.9.1\src\base\ftglyph.c
%cmpl% external\freetype-2.9.1\src\gzip\ftgzip.c
%cmpl% external\freetype-2.9.1\src\base\ftinit.c
%cmpl% external\freetype-2.9.1\src\lzw\ftlzw.c
%cmpl% external\freetype-2.9.1\src\base\ftstroke.c
%cmpl% external\freetype-2.9.1\src\base\ftsystem.c
%cmpl% external\freetype-2.9.1\src\smooth\smooth.c
%cmpl% external\freetype-2.9.1\src\base\ftbbox.c
%cmpl% external\freetype-2.9.1\src\base\ftbdf.c
%cmpl% external\freetype-2.9.1\src\base\ftcid.c
%cmpl% external\freetype-2.9.1\src\base\ftmm.c
%cmpl% external\freetype-2.9.1\src\base\ftpfr.c
%cmpl% external\freetype-2.9.1\src\base\ftsynth.c
%cmpl% external\freetype-2.9.1\src\base\fttype1.c
%cmpl% external\freetype-2.9.1\src\base\ftwinfnt.c
%cmpl% external\freetype-2.9.1\src\base\ftgxval.c
%cmpl% external\freetype-2.9.1\src\base\ftotval.c
%cmpl% external\freetype-2.9.1\src\base\ftpatent.c
%cmpl% external\freetype-2.9.1\src\pcf\pcf.c
%cmpl% external\freetype-2.9.1\src\pfr\pfr.c
%cmpl% external\freetype-2.9.1\src\psaux\psaux.c
%cmpl% external\freetype-2.9.1\src\pshinter\pshinter.c
%cmpl% external\freetype-2.9.1\src\psnames\psmodule.c
%cmpl% external\freetype-2.9.1\src\raster\raster.c
%cmpl% external\freetype-2.9.1\src\sfnt\sfnt.c
%cmpl% external\freetype-2.9.1\src\truetype\truetype.c
%cmpl% external\freetype-2.9.1\src\type1\type1.c
%cmpl% external\freetype-2.9.1\src\cid\type1cid.c
%cmpl% external\freetype-2.9.1\src\type42\type42.c
%cmpl% external\freetype-2.9.1\src\winfonts\winfnt.c
%cmpl% SDL_ttf.c
lib.exe /OUT:"SDL2_ttf.lib" /NOLOGO *.obj
move SDL2_ttf.lib ..\..\dist\lib\x86\
del *.obj
