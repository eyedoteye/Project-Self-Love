#include "renderer.h"
#include "renderer_api.h"
// Note: Pixels are ints between 0 to ScreenDimension.
// Vertices are floats between -1 to 1.
inline void NormalizePixelsToVertex(
  float X1, float Y1,
  int ScreenWidth, int ScreenHeight,
  float *Vertex)
{
  Vertex[0] = (X1 / (float)ScreenWidth) * 2.f - 1.f;
  Vertex[1] = ((ScreenHeight - Y1) / (float)ScreenHeight) * 2.f - 1.f;
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

void AddSemicircleToDebugFanBuffer(
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

    X - RotatedHalfVector[0],
    Y + RotatedHalfVector[1],

    X - RotatedHalfVector[0],
    Y - RotatedHalfVector[1],

    X + RotatedHalfVector[0],
    Y - RotatedHalfVector[1],
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
  AddSemicircleToDebugFanBuffer(
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

void DrawDebugBuffers(renderer_memory * RendererMemory)
{
  if(1) // Purpose: Draw Debug Fan Buffer
  {
    glBindVertexArray(RendererMemory->DebugFanVAO);
    {
      fan_buffer* DebugFanBuffer = &RendererMemory->DebugFanBuffer;

      if(0) // Purpose: Shift Up The Colors Randomly
      {
        float *ColorBuffer = DebugFanBuffer->ColorBuffer;
        for(int BufferIndex = 1; BufferIndex < DebugFanBuffer->NextIndex; ++BufferIndex)
        {
          ColorBuffer[BufferIndex] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .8f;
          if(ColorBuffer[BufferIndex] > 1.f)
          {
            ColorBuffer[BufferIndex] = 1.f;
          }
          else if(ColorBuffer[BufferIndex] < -1.f)
          {
            ColorBuffer[BufferIndex] = -1.f;
          }
        }
        ColorBuffer[0] += (rand() / (float)RAND_MAX * 2.f - 1.f) * .41f;
        if(ColorBuffer[0] > 1.f)
        {
          ColorBuffer[0] = 1.f;
        }
        else if(ColorBuffer[0] < -1.f)
        {
          ColorBuffer[0] = -1.f;
        }
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

  if(1) // Purpose: Draw Debug Line Buffer
  {
    glBindVertexArray(RendererMemory->DebugLineVAO);
    {
      line_buffer* DebugLineBuffer = &RendererMemory->DebugLineBuffer;

      if(0) // Purpose: Shift Up The Lines Randomly
      {
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

      glDrawArrays(GL_LINES, 0, DebugLineBuffer->NextIndex / 3);
    }
  }

  if(1) // Purpose: Clear debug buffers
  {
    RendererMemory->DebugLineBuffer.NextIndex = 0;

    RendererMemory->DebugFanBuffer.Count = 0;
    RendererMemory->DebugFanBuffer.NextIndex = 0;
  }
}
