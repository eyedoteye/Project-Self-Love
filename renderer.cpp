#include <assert.h>
#define ASSERT(...) assert(__VA_ARGS__)

#include <stdio.h>

#define internal static
#define global_variable static
#define local_persist static

#include "renderer.h"

#include <SDL2/SDL_video.h>
#define GLEW_STATIC
#include <GL/glew.h>

#include "game.h"

struct renderer_memory
{
  SDL_Window *Window;
  SDL_GLContext GLContext;
};

extern "C"
LOAD_RENDERER(LoadRenderer)
{
  renderer_memory *RendererMemory = (renderer_memory *)AllocatedMemory;

  RendererMemory->Window = Window;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  RendererMemory->GLContext = SDL_GL_CreateContext(RendererMemory->Window);

  // Fun: Why is GLEW so weird?
  //glewExperimental = GL_TRUE;
  int GLEW_InitStatus = glewInit();
  printf("Error: %s\n", glewGetErrorString(GLEW_InitStatus));
  ASSERT(GLEW_InitStatus == GLEW_OK);
}

extern "C"
RELOAD_RENDERER(ReloadRenderer)
{
  (void)0;
}

extern "C"
RENDER_GAME(RenderGame)
{
  renderer_memory *RendererMemory = (renderer_memory *)AllocatedMemory;

  SDL_GL_SwapWindow(RendererMemory->Window);
}
