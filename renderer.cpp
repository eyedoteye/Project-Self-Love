#include <assert.h>
#define ASSERT(...) assert(__VA_ARGS__)

#include <stdio.h>

#define internal static
#define global_variable static
#define local_persist static

#include <SDL2/SDL_video.h>
#define GLEW_STATIC
#include <GL/glew.h>

#include "renderer.h"
#include "renderer_debug_tools.cpp"
#include "shader_object.cpp"
#include "game.h"

#include <stdlib.h>

struct renderer_memory
{
  size_t NextMemoryPosition;

  SDL_Window *Window;
  SDL_GLContext GLContext;

  line_buffer DebugLineBuffer;
  GLuint DebugLineVAO;
  GLuint DebugLineVertexVBO;
  GLuint DebugLineColorVBO;
};

#define DEBUG_LINE_BUFFER_ARRAY_SIZE 300

void DrawDebugLineVAO(
  line_buffer DebugLineBuffer,
  GLuint DebugLineVAO
)
{
  glBindVertexArray(DebugLineVAO);
  {
  } glBindVertexArray(0);
}

global_variable float TestLineVertices[] =
{
  .8f, .5f, 0.f,
  -.5f, -.5f, 0.f,
  
  -.6f, 0.f, 0.f,
  .6f, 0.f, 0.f
};
global_variable float TestLineColors[] =
{
  1.f, 0.f, 0.f,
  0.f, 0.f, 1.f,

  1.f, 1.f, 0.f,
  0.f, 1.f, 1.f
};

global_variable shader DebugShader;

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


  size_t *NextMemoryPosition = &RendererMemory->NextMemoryPosition;
  *NextMemoryPosition = (size_t)RendererMemory + sizeof(renderer_memory);

  line_buffer *DebugLineBuffer = &RendererMemory->DebugLineBuffer;

  DebugLineBuffer->VertexBuffer = (float*)(*NextMemoryPosition);
  *NextMemoryPosition += DEBUG_LINE_BUFFER_ARRAY_SIZE * sizeof(float);

  RendererMemory->DebugLineBuffer.ColorBuffer = (float*)(*NextMemoryPosition);
  *NextMemoryPosition += DEBUG_LINE_BUFFER_ARRAY_SIZE * sizeof(float);

  DebugLineBuffer->NextIndex = 0;
  // Test: Memory Test
  //for(int i = 0; i < DEBUG_LINE_BUFFER_ARRAY_SIZE; ++i)
  //{
  //  DebugLineBuffer->VertexBuffer[i] = i;
  //  DebugLineBuffer->ColorBuffer[i] = i;
  //}

  glGenVertexArrays(1, &RendererMemory->DebugLineVAO);
  glGenBuffers(1, &RendererMemory->DebugLineVertexVBO);
  glGenBuffers(1, &RendererMemory->DebugLineColorVBO);

  AddVerticesToDebugLineVBO(
    &RendererMemory->DebugLineBuffer,
    TestLineVertices, TestLineColors,
    4 
  );

  DebugShader.Name = "DebugPrimitive";
  {
    DebugShader.VertexSource =
      #include "debug_shader.vert"
  }
  DebugShader.GeometrySource = NULL;
  {
    DebugShader.FragmentSource =
      #include "debug_shader.frag"
  }
  compileShader(&DebugShader);
  linkShader(&DebugShader);

  glBindVertexArray(RendererMemory->DebugLineVAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugLineVertexVBO);
    {
      glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * DebugLineBuffer->NextIndex,
        DebugLineBuffer->VertexBuffer,
        GL_STATIC_DRAW);
      glVertexAttribPointer(
        0,
        3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
        (GLvoid*)0);
      glEnableVertexAttribArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugLineColorVBO);
    {
      glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * DebugLineBuffer->NextIndex,
        DebugLineBuffer->ColorBuffer,
        GL_STATIC_DRAW);
      glVertexAttribPointer(
        1,
        3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
        (GLvoid*)0);
      glEnableVertexAttribArray(1);
    }
  }

  srand(0);
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

  if(1) // Purpose: Draw Debug Line Buffer
  {
    line_buffer* DebugLineBuffer = &RendererMemory->DebugLineBuffer;
    
    glUseProgram(DebugShader.ID);
    glBindVertexArray(RendererMemory->DebugLineVAO);
    {
      for(int BufferIndex = 0; BufferIndex < DebugLineBuffer->NextIndex; ++BufferIndex)
      {
        float *VertexBuffer = DebugLineBuffer->VertexBuffer;
        VertexBuffer[BufferIndex] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .01f;
        if(VertexBuffer[BufferIndex] > 1.f)
        {
          VertexBuffer[BufferIndex] = 1.f;
        }
        else if(VertexBuffer[BufferIndex] < -1.f)
        {
          VertexBuffer[BufferIndex] = -1.f;
        }

        float *ColorBuffer = DebugLineBuffer->ColorBuffer;
        ColorBuffer[BufferIndex] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .01f;
        if(ColorBuffer[BufferIndex] > 1.f)
        {
          ColorBuffer[BufferIndex] = 1.f;
        }
        else if(ColorBuffer[BufferIndex] < -1.f)
        {
          ColorBuffer[BufferIndex] = -1.f;
        }
      }

      glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugLineVertexVBO);
      {
        glBufferData(
          GL_ARRAY_BUFFER,
          sizeof(float) * DebugLineBuffer->NextIndex,
          DebugLineBuffer->VertexBuffer,
          GL_STATIC_DRAW);
      }
      glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugLineColorVBO);
      {
      glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * DebugLineBuffer->NextIndex,
        DebugLineBuffer->ColorBuffer,
        GL_STATIC_DRAW);
      }

      glDrawArrays(GL_LINES, 0, 4);
    }
    glBindVertexArray(0);
  }

  SDL_GL_SwapWindow(RendererMemory->Window);
}
