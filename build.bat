@echo off

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2>NUL

set IncludesDirectory=..\Includes
set SDL2LibDirectory=..\Libs\SDL2
set GLEWLibDirectory=..\Libs\GL

set CommonCompilerFlags= ^
  -MTd -nologo -GR- -Oi -W4 -FC -Z7 -wd4201 -wd4100

set PlatformCompilerFlags=%CommonCompilerFlags% ^
  -I %IncludesDirectory%

set PlatformLinkerFlags= ^
  -INCREMENTAL:no user32.lib gdi32.lib winmm.lib ^
  -LIBPATH:%SDL2LibDirectory% SDL2.lib SDL2main.lib ^
  opengl32.lib ^
  -LIBPATH:%GLEWLibDirectory% glew32sd.lib

echo Compiling Game DLL
echo DLL LOCK > lock.tmp
cl %CommonCompilerFlags% ..\game.cpp -LD ^
   /link -INCREMENTAL:no -opt:ref -PDB:game_%random%.pdb ^
         -EXPORT:LoadGame -EXPORT:ReloadGame -EXPORT:UpdateAndRenderGame
del lock.tmp

echo Compiling Platform EXE
cl %PlatformCompilerFlags% ..\sdl_main.cpp ^
   /link %PlatformLinkerFlags% -SUBSYSTEM:windows ^
   -OUT:main.exe

echo Copying Required LIBS
copy %SDL2LibDirectory%\SDL2.dll SDL2.dll

popd
