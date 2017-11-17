#include <assert.h>
#define ASSERT(...) assert(__VA_ARGS__)

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"

#define internal static
#define global_variable static
#define local_persist static

#include <SDL2/SDL_video.h>
#define GLEW_STATIC
#include <GL/glew.h>

#include "renderer.h"
#include "shader_object.cpp"

global_variable renderer_memory *GlobalRendererMemory;

// Note: Pixels are ints between 0 to ScreenDimension.
// Vertices are floats between -1 to 1.
inline void NormalizePixelsToVertex(
  float X1, float Y1,
  int ScreenWidth, int ScreenHeight,
  float *Vertex)
{
  Vertex[0] = (X1 / (float)ScreenWidth) * 2.f - 1.f;
  Vertex[1] = (Y1 / (float)ScreenHeight) * 2.f - 1.f;
}

inline void CopyVec3ToBuffer(
  float *Buffer,
  int StartIndex,
  float Vec3X, float Vec3Y, float Vec3Z 
)
{
  Buffer[StartIndex] = Vec3X;
  Buffer[StartIndex + 1] = Vec3Y;
  Buffer[StartIndex + 2] = Vec3Z;
}

//void AddVerticesToDebugLineBuffer(
//  line_buffer *DebugLineBuffer,
//  float *Vertices, float *Colors,
//  int NumberOfVertices)
//{
//  float *VertexBuffer = DebugLineBuffer->VertexBuffer;
//  float *ColorBuffer = DebugLineBuffer->ColorBuffer;
//  int *NextBufferIndex = &DebugLineBuffer->NextIndex;
//  int NumberOfFloats = NumberOfVertices * 3;
//
//  for(int vertexIndex = 0; vertexIndex < NumberOfFloats; ++vertexIndex)
//  {
//    VertexBuffer[*NextBufferIndex] = Vertices[vertexIndex];
//    ColorBuffer[*NextBufferIndex] = Colors[vertexIndex];
//    ++(*NextBufferIndex);
//  }
//}

void AddLineToDebugLineBuffer(
  line_buffer *DebugLineBuffer,
  float X1, float Y1,
  float X2, float Y2,
  float R, float G, float B
)
{
  float Color[3] =
  {
    R / 255.f,
    G / 255.f,
    B / 255.f
  };

  float *VertexBuffer = DebugLineBuffer->VertexBuffer;
  float *ColorBuffer = DebugLineBuffer->ColorBuffer;
  int *NextBufferIndex = &DebugLineBuffer->NextIndex;

  float StartVertex[2];
  NormalizePixelsToVertex(
    X1, Y1,
    GlobalScreenWidth, GlobalScreenHeight,
    StartVertex 
  );
  float EndVertex[2];
  NormalizePixelsToVertex(
    X2, Y2,
    GlobalScreenWidth, GlobalScreenHeight,
    EndVertex
  );

  CopyVec3ToBuffer(
    VertexBuffer,
    *NextBufferIndex,
    StartVertex[0], StartVertex[1], 0.f
  );
  CopyVec3ToBuffer(
    ColorBuffer,
    *NextBufferIndex,
    Color[0], Color[1], Color[2]
  );
  *NextBufferIndex += 3;

  CopyVec3ToBuffer(
    VertexBuffer,
    *NextBufferIndex,
    EndVertex[0], EndVertex[1], 0.f
  );
  CopyVec3ToBuffer(
    ColorBuffer,
    *NextBufferIndex,
    Color[0], Color[1], Color[2]
  );
  *NextBufferIndex += 3;
}


void AddSemiCircleToDebugFanBuffer(
  fan_buffer *DebugFanBuffer,
  float X, float Y,
  float Radius,
  int Segments, int TotalSegments,
  float Angle,
  float R, float G, float B 
)
{
  float Color[3] =
  {
    R / 255.f,
    G / 255.f,
    B / 255.f
  };

  float *VertexBuffer = DebugFanBuffer->VertexBuffer;
  float *ColorBuffer = DebugFanBuffer->ColorBuffer;
  int *NextBufferIndex = &DebugFanBuffer->NextIndex;

  float RadianAngle = Angle * DEG2RAD_CONSTANT;

  float CenterVertex[2];
  NormalizePixelsToVertex(
    X, Y,
    GlobalScreenWidth, GlobalScreenHeight,
    CenterVertex
  );

  CopyVec3ToBuffer(
    VertexBuffer,
    *NextBufferIndex,
    CenterVertex[0], CenterVertex[1], 0.f
  );
  CopyVec3ToBuffer(
    ColorBuffer,
    *NextBufferIndex,
    Color[0], Color[1], Color[2]
  );
  *NextBufferIndex += 3;

  for(int VertexNumber = 0; VertexNumber <= Segments; ++VertexNumber)
  {
    float Position[2];
    Position[0] = X + cosf(RadianAngle + VertexNumber / (float)TotalSegments * PI * 2) * Radius;
    Position[1] = Y + sinf(RadianAngle + VertexNumber / (float)TotalSegments * PI * 2) * Radius;

    float Vertex[2];
    NormalizePixelsToVertex(
      Position[0], Position[1],
      GlobalScreenWidth, GlobalScreenHeight,
      Vertex
    );

    CopyVec3ToBuffer(
      VertexBuffer,
      *NextBufferIndex,
      Vertex[0], Vertex[1], 0.f
    );

    CopyVec3ToBuffer(
      ColorBuffer,
      *NextBufferIndex,
      Color[0], Color[1], Color[2]
    );
    *NextBufferIndex += 3;
  }

  int *Count = &DebugFanBuffer->Count;
  // Add 1 for origin; Add 1 for last vertex
  // There's always 1 more vertex than segments
  DebugFanBuffer->Strides[*Count] = Segments + 2;
  *Count += 1;
}

void AddRectToDebugFanBuffer(
  fan_buffer *DebugFanBuffer,
  float X, float Y,
  float Width, float Height,
  float Angle,
  float R, float G, float B
)
{
  float Color[3] =
  {
    R / 255.f,
    G / 255.f,
    B / 255.f
  };

  float *VertexBuffer = DebugFanBuffer->VertexBuffer;
  float *ColorBuffer = DebugFanBuffer->ColorBuffer;
  int *NextBufferIndex = &DebugFanBuffer->NextIndex;

  float RadianAngle = Angle * DEG2RAD_CONSTANT;

  // Todo: Add screen resolution fixes to (X, Y) coordinates
  // Since the values are normalized to [-1, 1],
  // rotation also misplaces the vertex due to stretch/squash.
  float RotatedHalfVector[2] =
  {
    Width / 2.f * (cosf(RadianAngle) - sinf(RadianAngle)),
    Height / 2.f * (sinf(RadianAngle) + cosf(RadianAngle))
  };

  float Vertices[2 * 4] =
  {
    X + RotatedHalfVector[0],
    Y + RotatedHalfVector[1],

    X - RotatedHalfVector[1],
    Y + RotatedHalfVector[0],

    X - RotatedHalfVector[0],
    Y - RotatedHalfVector[1],

    X + RotatedHalfVector[1],
    Y - RotatedHalfVector[0],
  };

  for(int VertexNumber = 0; VertexNumber < 4 * 2; VertexNumber += 2)
  {
    float Vertex[2];
    NormalizePixelsToVertex(
      Vertices[VertexNumber], Vertices[VertexNumber + 1],
      GlobalScreenWidth, GlobalScreenHeight,
      Vertex
    );

    CopyVec3ToBuffer(
      VertexBuffer,
      *NextBufferIndex,
      Vertex[0], Vertex[1], 0.f
    );
    CopyVec3ToBuffer(
      ColorBuffer,
      *NextBufferIndex,
      Color[0], Color[1], Color[2]
    );
    *NextBufferIndex += 3;
  }

  int *Count = &DebugFanBuffer->Count;
  DebugFanBuffer->Strides[*Count] = 4;
  *Count += 1;
}

extern "C" ADD_LINE_TO_RENDERER(AddLineToRenderer)
{
  AddLineToDebugLineBuffer(
    &GlobalRendererMemory->DebugLineBuffer,
    X1, Y1,
    X2, Y2,
    R, G, B
  );
}

extern "C" ADD_SEMICIRCLE_TO_RENDERER(AddSemicircleToRenderer)
{
  AddSemiCircleToDebugFanBuffer(
    &GlobalRendererMemory->DebugFanBuffer,
    X, Y,
    Radius,
    Segments, TotalSegments,
    Angle,
    R, G, B
  );
}

extern "C" ADD_RECT_TO_RENDERER(AddRectToRenderer)
{
  AddRectToDebugFanBuffer(
    &GlobalRendererMemory->DebugFanBuffer,
    X, Y,
    Width, Height,
    Angle,
    R, G, B
  );
}

#define DEBUG_LINE_BUFFER_ARRAY_SIZE 300
#define DEBUG_FAN_BUFFER_ARRAY_SIZE 300
#define DEBUG_FAN_BUFFER_STRIDES_ARRAY_SIZE 100

global_variable shader DebugShader;

extern "C"
LOAD_RENDERER(LoadRenderer)
{
  srand(0); // Purpose: Seeding rand for testing

  renderer_memory *RendererMemory = (renderer_memory *)AllocatedMemory;
  GlobalRendererMemory = RendererMemory;

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


  if(1) // Purpose: Setup DebugLine related things.
  {
    line_buffer *DebugLineBuffer = &RendererMemory->DebugLineBuffer;

    DebugLineBuffer->VertexBuffer = (float*)(*NextMemoryPosition);
    *NextMemoryPosition += DEBUG_LINE_BUFFER_ARRAY_SIZE * sizeof(float);

    DebugLineBuffer->ColorBuffer = (float*)(*NextMemoryPosition);
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

    glBindVertexArray(RendererMemory->DebugLineVAO);
    {
      glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugLineVertexVBO);
      {
        glVertexAttribPointer(
          0,
          3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
          (GLvoid*)0);
        glEnableVertexAttribArray(0);
      }
      glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugLineColorVBO);
      {
        glVertexAttribPointer(
          1,
          3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
          (GLvoid*)0);
        glEnableVertexAttribArray(1);
      }
    }
  }

  if(1) // Purpose: Setup DebugFanBuffer
  {
    fan_buffer *DebugFanBuffer = &RendererMemory->DebugFanBuffer;

    DebugFanBuffer->VertexBuffer = (float*)(*NextMemoryPosition);
    *NextMemoryPosition += DEBUG_FAN_BUFFER_ARRAY_SIZE * sizeof(float);

    DebugFanBuffer->ColorBuffer = (float*)(*NextMemoryPosition);
    *NextMemoryPosition += DEBUG_FAN_BUFFER_ARRAY_SIZE * sizeof(float);

    DebugFanBuffer->Strides = (int*)(*NextMemoryPosition);
    *NextMemoryPosition += DEBUG_FAN_BUFFER_STRIDES_ARRAY_SIZE * sizeof(int);

    DebugFanBuffer->NextIndex = 0;
    DebugFanBuffer->Count = 0;

    glGenVertexArrays(1, &RendererMemory->DebugFanVAO);
    glGenBuffers(1, &RendererMemory->DebugFanVertexVBO);
    glGenBuffers(1, &RendererMemory->DebugFanColorVBO);

    glBindVertexArray(RendererMemory->DebugFanVAO);
    {
      glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugFanVertexVBO);
      {
        glVertexAttribPointer(
          0,
          3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
          (GLvoid*)0);
        glEnableVertexAttribArray(0);
      }
      glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugFanColorVBO);
      {
        glVertexAttribPointer(
          1,
          3, GL_FLOAT, GL_FALSE, sizeof(float) * 3,
          (GLvoid*)0);
        glEnableVertexAttribArray(1);
      }
    }
  }

  if(1) // Purpose: Setup Debug Shader
  {
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
  }
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
    glUseProgram(DebugShader.ID);
    glBindVertexArray(RendererMemory->DebugLineVAO);
    {
      line_buffer* DebugLineBuffer = &RendererMemory->DebugLineBuffer;

      float *VertexBuffer = DebugLineBuffer->VertexBuffer;
      float *ColorBuffer = DebugLineBuffer->ColorBuffer;
      for(int BufferIndex = 0; BufferIndex < DebugLineBuffer->NextIndex; ++BufferIndex)
      {
        VertexBuffer[BufferIndex] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .01f;
        if(VertexBuffer[BufferIndex] > 1.f)
        {
          VertexBuffer[BufferIndex] = 1.f;
        }
        else if(VertexBuffer[BufferIndex] < -1.f)
        {
          VertexBuffer[BufferIndex] = -1.f;
        }

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
  }

  if(1) // Purpose: Draw Debug Fan Buffer
  {
    glBindVertexArray(RendererMemory->DebugFanVAO);
    {
      fan_buffer* DebugFanBuffer = &RendererMemory->DebugFanBuffer;

      float *ColorBuffer = DebugFanBuffer->ColorBuffer;
      for(int BufferIndex = 1; BufferIndex < DebugFanBuffer->NextIndex; ++BufferIndex)
      {
        ColorBuffer[BufferIndex] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .02f;
        if(ColorBuffer[BufferIndex] > 1.f)
        {
          ColorBuffer[BufferIndex] = 1.f;
        }
        else if(ColorBuffer[BufferIndex] < -1.f)
        {
          ColorBuffer[BufferIndex] = -1.f;
        }
      }
      ColorBuffer[0] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .01f;
      if(ColorBuffer[0] > 1.f)
      {
        ColorBuffer[0] = 1.f;
      }
      else if(ColorBuffer[0] < -1.f)
      {
        ColorBuffer[0] = -1.f;
      }

      glBindVertexArray(RendererMemory->DebugFanVAO);
      {
        glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugFanVertexVBO);
        {
          glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * DebugFanBuffer->NextIndex,
            DebugFanBuffer->VertexBuffer,
            GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ARRAY_BUFFER, RendererMemory->DebugFanColorVBO);
        {
          glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * DebugFanBuffer->NextIndex,
            DebugFanBuffer->ColorBuffer,
            GL_STATIC_DRAW);
        }
      }

      int *Strides = RendererMemory->DebugFanBuffer.Strides;
      int ShapeCount = RendererMemory->DebugFanBuffer.Count;

      int BufferIndex = 0;
      for(int ShapeIndex = 0; ShapeIndex < ShapeCount; ++ShapeIndex)
      {
        int VertexCount = Strides[ShapeIndex];
        glDrawArrays(GL_TRIANGLE_FAN, BufferIndex, VertexCount);

        BufferIndex += VertexCount;
      }
    }
  }

  SDL_GL_SwapWindow(RendererMemory->Window);
}
