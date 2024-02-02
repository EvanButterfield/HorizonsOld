@echo off
IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM -O2 makes the compile REALLY slow
set CompilerFlags=-UTF8 -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4221 -wd4505 -wd4201 -wd4100 -wd4189 -wd4115 -wd4101 -wd4996 -FAsc -Z7 -DHORIZONS_INTERNAL=1 -DHORIZONS_SLOW=1 -DUNICODE=1
set LinkerFlags=-incremental:no -opt:ref kernel32.lib user32.lib gdi32.lib winmm.lib d3d11.lib d3dcompiler.lib dxguid.lib dxgi.lib xaudio2.lib ole32.lib

del *.pdb > NUL 2> NUL
cl %CompilerFlags% W:\new-horizons\code\horizons.cpp -LD /link -incremental:no -opt:ref -Export:GameUpdateAndRender
REM cl -GS- -Gs9999999 %CompilerFlags% W:\new-horizons\code\win32_horizons.c /link -nodefaultlib -subsystem:windows %LinkerFlags%
popd
