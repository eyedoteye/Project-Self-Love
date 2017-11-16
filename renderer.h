#include <SDL2/SDL_video.h>

typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef signed char GLbyte;
typedef short GLshort;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned long GLulong;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;

#define LOAD_RENDERER(name) void name(void* AllocatedMemory, SDL_Window *Window)
typedef LOAD_RENDERER(load_renderer);

#define RELOAD_RENDERER(name) void name(void* AllocatedMemory, SDL_Window *Window)
typedef RELOAD_RENDERER(reload_renderer);

#define RENDER_GAME(name) void name(void* AllocatedMemory)
typedef RENDER_GAME(render_game);


#define DEG2RAD_CONSTANT PI / 180.f


global_variable int GlobalScreenWidth = 1280;
global_variable int GlobalScreenHeight = 1024;


struct line_buffer
{
  GLfloat *VertexBuffer;
  GLfloat *ColorBuffer;
  int NextIndex;
};

struct fan_buffer
{
  GLfloat *VertexBuffer;
  GLfloat *ColorBuffer;
  int *Strides;
  int Count;
  int NextIndex;
};

struct renderer_memory
{
  size_t NextMemoryPosition;

  SDL_Window *Window;
  SDL_GLContext GLContext;

  line_buffer DebugLineBuffer;
  GLuint DebugLineVAO;
  GLuint DebugLineVertexVBO;
  GLuint DebugLineColorVBO;

  fan_buffer DebugFanBuffer;
  GLuint DebugFanVAO;
  GLuint DebugFanVertexVBO;
  GLuint DebugFanColorVBO;
};

#define ADD_LINE_TO_RENDERER(name) void name(\
  float X1, float Y1,\
  float X2, float Y2,\
  float R, float G, float B\
)
typedef ADD_LINE_TO_RENDERER(add_line_to_renderer);

#define ADD_SEMICIRCLE_TO_RENDERER(name) void name(\
  float X, float Y,\
  float Radius,\
  int Segments, int TotalSegments,\
  float Angle,\
  float R, float G, float B\
)
typedef ADD_SEMICIRCLE_TO_RENDERER(add_semicircle_to_renderer);

#define ADD_RECT_TO_RENDERER(name) void name(\
  float X, float Y,\
  float Width, float Height,\
  float Angle,\
  float R, float G, float B\
)
typedef ADD_RECT_TO_RENDERER(add_rect_to_renderer);