@echo off

IF "%1"=="x86" GOTO StartBatch
IF "%1"=="x64" GOTO StartBatch 
GOTO WrongArgs
:StartBatch

echo Compiling For %1

IF NOT EXIST build mkdir build
pushd build

del *.pdb > NUL 2>NUL

set IncludesDirectory=..\Includes
IF "%1"=="x86" GOTO x86
IF "%1"=="x64" GOTO x64

:x86
set SDL2LibDirectory=..\Libs\SDL2\x86-lib
set GLEWLibDirectory=..\Libs\GL\Win32
GOTO ContinueBatch

:x64
set SDL2LibDirectory=..\Libs\SDL2\x64-lib
set GLEWLibDirectory=..\Libs\GL\x64
GOTO ContinueBatch

:ContinueBatch
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

IF "%2"=="R" GOTO CompileRenderer

echo Compiling Game DLL
echo GAME DLL LOCK > lock.tmp
cl %CommonCompilerFlags% ..\game.cpp -LD ^
   /link %CommonLinkerFlags% ^
         -PDB:game_%random%.pdb ^
         -EXPORT:LoadGame -EXPORT:ReloadGame -EXPORT:UpdateGame
del lock.tmp
IF "%2"=="G" GOTO End  

:CompileRenderer
echo Compiling Renderer DLL
echo RENDERER DLL LOCK > lock.tmp
cl %RendererCompilerFlags% ..\renderer.cpp -LD ^
   /link %RendererLinkerFlags% ^
         -PDB:renderer_%random%.pdb ^
         -NODEFAULTLIB:msvcrtd.lib ^
         -EXPORT:LoadRenderer -EXPORT:ReloadRenderer -Export:RenderGame
del lock.tmp
IF "%2"=="R" GOTO End
IF "%2"=="GR" GOTO End

echo Compiling Platform EXE
cl %PlatformCompilerFlags% ..\sdl_main.cpp ^
   /link %PlatformLinkerFlags% ^
         -SUBSYSTEM:windows ^
         -OUT:main.exe

echo Copying Required LIBS
copy %SDL2LibDirectory%\SDL2.dll SDL2.dll

popd

GOTO End

:WrongArgs
  echo Please supply either: x86 x64

:End
