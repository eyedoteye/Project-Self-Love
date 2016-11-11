@echo off

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2>NUL

set SDL2IncludeDirectory=..\SDL2\include
set SDL2LibDirectory=..\SDL2\x86-lib

set CommonCompilerFlags=-MTd -nologo -GR- -EHa- -Oi -WX -W4 -FC -Z7 -wd4201 -wd4100 -I %SDL2IncludeDirectory%
set CommonLinkerFlags=-INCREMENTAL:NO -LIBPATH:%SDL2LibDirectory% user32.lib gdi32.lib winmm.lib SDL2.lib SDL2main.lib

cl %CommonCompilerFlags% ..\game.cpp -LD /link -INCREMENTAL:no -opt:ref -PDB:game_%random%.pdb -EXPORT:LoadGame -EXPORT:UpdateAndRenderGame
cl %CommonCompilerFlags% ..\sdl_main.cpp /link %CommonLinkerFlags% /SUBSYSTEM:windows,5.1

IF NOT EXIST SDL2.dll copy %SDL2LibDirectory%\SDL2.dll SDL2.dll

popd