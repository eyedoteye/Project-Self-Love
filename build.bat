@echo off

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2>NUL

set IncludesDirectory=..\Includes
set SDL2LibDirectory=..\Libs\SDL2
set GLEWLibDirectory=..\Libs\GL

set CompilerFlags=-MTd -nologo -GR- -Oi -W4 -FC -Z7 -wd4201 -wd4100
set CompilerFlags=-I %IncludesDirectory% %CompilerFlags%

set LinkerFlags=-INCREMENTAL:NO user32.lib gdi32.lib winmm.lib
set LinkerFlags=-LIBPATH:%SDL2LibDirectory% SDL2.lib SDL2main.lib %LinkerFlags%

echo Compiling Game DLL
echo DLL LOCK > lock.tmp
cl %CompilerFlags% ..\game.cpp -LD /link -INCREMENTAL:no -opt:ref -PDB:game_%random%.pdb -EXPORT:LoadGame -EXPORT:ReloadGame -EXPORT:UpdateAndRenderGame
del lock.tmp

echo Compiling Platform EXE
cl %CompilerFlags% ..\sdl_main.cpp /link %LinkerFlags% -SUBSYSTEM:windows

echo Copying Required LIBS
copy %SDL2LibDirectory%\SDL2.dll SDL2.dll

popd
