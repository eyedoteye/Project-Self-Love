#include <GL/glew.h> // Ignored
#include "common.h" // Ignored

global_variable GLuint DebugLineColorBuffer;

struct line_buffer
{
  GLfloat *VertexBuffer;
  GLfloat *ColorBuffer;
  int NextIndex;
};

void AddVerticesToDebugLineVBO(
  line_buffer* DebugLineBuffer,
  float* Vertices, float* Colors,
  int NumberOfVertices)
{
  GLfloat *VertexBuffer = DebugLineBuffer->VertexBuffer;
  GLfloat *ColorBuffer = DebugLineBuffer->ColorBuffer;
  int *NextBufferIndex = &(DebugLineBuffer->NextIndex);
  int NumberOfFloats = NumberOfVertices * 3;
  for(int vertexIndex = 0; vertexIndex < NumberOfFloats; ++vertexIndex)
  {
    VertexBuffer[*NextBufferIndex] = Vertices[vertexIndex];
    ColorBuffer[*NextBufferIndex] = Colors[vertexIndex];
    ++(*NextBufferIndex);
  }
}