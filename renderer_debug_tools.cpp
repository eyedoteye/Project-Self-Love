#include <GL/glew.h> // Ignored
#include "common.h" // Ignored

global_variable int GlobalScreenWidth = 1280;
global_variable int GlobalScreenHeight = 1024;

inline void NormalizePixelsToVertex(
  float X1, float Y1,
  int ScreenWidth, int ScreenHeight,
  float* Vertex)
{
  Vertex[0] = (X1 / (float)ScreenWidth) * 2.f - 1.f;
  Vertex[1] = (Y1 / (float)ScreenHeight) * 2.f - 1.f;
}

inline void CopyVec3ToBuffer(
  float* Buffer,
  int StartIndex,
  float Vec3X, float Vec3Y, float Vec3Z 
)
{
  Buffer[StartIndex] = Vec3X;
  Buffer[StartIndex + 1] = Vec3Y;
  Buffer[StartIndex + 2] = Vec3Z;
}


struct line_buffer
{
  float *VertexBuffer;
  float *ColorBuffer;
  int NextIndex;
};

void AddVerticesToDebugLineBuffer(
  line_buffer *DebugLineBuffer,
  float *Vertices, float *Colors,
  int NumberOfVertices)
{
  float *VertexBuffer = DebugLineBuffer->VertexBuffer;
  float *ColorBuffer = DebugLineBuffer->ColorBuffer;
  int *NextBufferIndex = &DebugLineBuffer->NextIndex;
  int NumberOfFloats = NumberOfVertices * 3;

  for(int vertexIndex = 0; vertexIndex < NumberOfFloats; ++vertexIndex)
  {
    VertexBuffer[*NextBufferIndex] = Vertices[vertexIndex];
    ColorBuffer[*NextBufferIndex] = Colors[vertexIndex];
    ++(*NextBufferIndex);
  }
}

void AddLineToDebugLineBuffer(
  line_buffer *DebugLineBuffer,
  float X1, float Y1,
  float X2, float Y2,
  int R, int G, int B
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

struct fan_buffer
{
  float *VertexBuffer;
  float *ColorBuffer;
  int *Strides;
  int Count;
  int NextIndex;
};

#define DEG2RAD_CONSTANT PI / 180.f

void AddSemiCircleToDebugFanBuffer(
  fan_buffer *DebugFanBuffer,
  float X, float Y,
  float Radius,
  int Segments, int TotalSegments,
  float Angle,
  int R, int G, int B 
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
  int R, int G, int B
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