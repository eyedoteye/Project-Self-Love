#include <GL/glew.h> // Ignored
#include "common.h" // Ignored

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

struct fan_buffer
{
  float *VertexBuffer;
  float *ColorBuffer;
  int *Strides;
  int Count;
  int NextIndex;
};

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

#define DEG2RAD_CONSTANT PI / 180.f
void AddSemiCircleToDebugFanBuffer(
  fan_buffer *DebugFanBuffer,
  float X, float Y,
  float Radius,
  int Segments, int TotalSegments,
  float Angle,
  float *Color
)
{
  float *VertexBuffer = DebugFanBuffer->VertexBuffer;
  float *ColorBuffer = DebugFanBuffer->ColorBuffer;
  int *NextBufferIndex = &DebugFanBuffer->NextIndex;

  float RadianAngle = Angle * DEG2RAD_CONSTANT;

  CopyVec3ToBuffer(
    VertexBuffer,
    *NextBufferIndex,
    X, Y, 0.f
  );
  CopyVec3ToBuffer(
    ColorBuffer,
    *NextBufferIndex,
    Color[0], Color[1], Color[2]
  );
  *NextBufferIndex += 3;

  float Position[2];
  Position[0] = X + cosf(RadianAngle) * Radius;
  Position[1] = Y + sinf(RadianAngle) * Radius;

  for(int VertexNumber = 0; VertexNumber <= Segments; VertexNumber++)
  {
    Position[0] = X + cosf(RadianAngle + VertexNumber / (float)TotalSegments * PI * 2) * Radius;
    Position[1] = Y + sinf(RadianAngle + VertexNumber / (float)TotalSegments * PI * 2) * Radius;

    CopyVec3ToBuffer(
      VertexBuffer,
      *NextBufferIndex,
      Position[0], Position[1], 0.f
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