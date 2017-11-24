#pragma once
#include <SDL2/SDL_video.h>


#define LOAD_RENDERER(name) void name(void* AllocatedMemory, SDL_Window *Window)
typedef LOAD_RENDERER(load_renderer);

#define RELOAD_RENDERER(name) void name(void* AllocatedMemory, SDL_Window *Window)
typedef RELOAD_RENDERER(reload_renderer);

#define RENDER_GAME(name) void name(void* AllocatedMemory)
typedef RENDER_GAME(render_game);


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