#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define internal static
#define global_variable static
#define local_persist static

#define SQRT2	1.41421356237f
#define PI		3.14159265359f
#define DEG2RAD_CONSTANT	PI / 180.f
#define RAD2DEG_CONSTANT	180.f / PI

#define CLIP(X, A, B) ((X < A) ? A : ((X > B) ? B : X))
#define ABS(X) (X < 0 ? -X : X)
#define LERP(A, B, T) ((1-T)*A + T*B)

// Todo: Move to a seperate header?
#define DEBUG_PRINT(name) void name( \
	char* OutputString,			\
	...											\
)
typedef DEBUG_PRINT(debug_print);

#define DEBUG_DRAW_SEMI_CIRCLE(name) void name( \
	float X, float Y,                 \
	float Radius,                     \
	int Segments, int TotalSegments,  \
	float Angle                       \
)
typedef DEBUG_DRAW_SEMI_CIRCLE(debug_draw_semi_circle);

#define DEBUG_DRAW_CIRCLE(name) void name(  \
  float X, float Y, \
  float Radius,     \
  int Segments      \
)
typedef DEBUG_DRAW_CIRCLE(debug_draw_circle);

#define DEBUG_DRAW_TRIANGLE(name) void name(  \
  float X, float Y, \
  float Angle,      \
  float HalfHeight  \
)
typedef DEBUG_DRAW_TRIANGLE(debug_draw_triangle);

#define DEBUG_FILL_BOX(name) void name( \
  float X, float Y,         \
  float Width, float Height \
)
typedef DEBUG_FILL_BOX(debug_fill_box);

#define DEBUG_DRAW_BOX(name) void name( \
  float X, float Y,         \
  float Width, float Height \
)
typedef DEBUG_DRAW_BOX(debug_draw_box);

#define DEBUG_DRAW_LINE(name) void name( \
  float X1, float Y1, \
  float X2, float Y2  \
)
typedef DEBUG_DRAW_LINE(debug_draw_line);

#define DEBUG_SET_COLOR(name) void name( \
  int R, int G, int B, int A)
typedef DEBUG_SET_COLOR(debug_set_color);

struct debug_tools
{
	debug_print *Print;
  debug_draw_semi_circle *DrawSemiCircle;
  debug_draw_circle *DrawCircle;
  debug_draw_triangle *DrawTriangle;
  debug_fill_box *FillBox;
  debug_draw_box *DrawBox;
  debug_draw_line *DrawLine;
  debug_set_color *SetColor;
};
global_variable debug_tools *GlobalDebugTools;
