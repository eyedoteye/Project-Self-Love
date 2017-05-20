@echo off

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2>NUL

set IncludesDirectory=..\Includes
set SDL2LibDirectory=..\Libs\SDL2
set GLEWLibDirectory=..\Libs\GL

set CommonCompilerFlags=-MTd -nologo -GR- -Oi -W4 -FC -Z7 -wd4201 -wd4100

set PlatformCompilerFlags=%CommonCompilerFlags% ^
  -I %IncludesDirectory%

set RendererCompilerFlags=%PlatformCompilerFlags%

set CommonLinkerFlags= ^
  -INCREMENTAL:no -opt:ref

set RendererLinkerFlags=%CommonLinkerFlags% ^
  opengl32.lib ^
  -LIBPATH:%GLEWLibDirectory% glew32sd.lib ^
  -LIBPATH:%SDL2LibDirectory% SDL2.lib

set PlatformLinkerFlags=%CommonLinkerFlags% ^
  user32.lib gdi32.lib winmm.lib ^
  -LIBPATH:%SDL2LibDirectory% SDL2.lib SDL2main.lib

echo Compiling Game DLL
echo GAME DLL LOCK > lock.tmp
cl %CommonCompilerFlags% ..\game.cpp -LD ^
   /link %CommonLinkerFlags% ^
         -PDB:game_%random%.pdb ^
         -EXPORT:LoadGame -EXPORT:ReloadGame -EXPORT:UpdateGame
del lock.tmp

echo Compiling Renderer DLL
echo RENDERER DLL LOCK > lock.tmp
cl %RendererCompilerFlags% ..\renderer.cpp -LD ^
   /link %RendererLinkerFlags% ^
         -PDB:renderer_%random%.pdb ^
         -EXPORT:LoadRenderer -EXPORT:ReloadRenderer -Export:RenderGame
del lock.tmp

echo Compiling Platform EXE
cl %PlatformCompilerFlags% ..\sdl_main.cpp ^
   /link %PlatformLinkerFlags% -SUBSYSTEM:windows ^
         -OUT:main.exe

echo Copying Required LIBS
copy %SDL2LibDirectory%\SDL2.dll SDL2.dll

popd
