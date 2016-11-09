#pragma once

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270
#define CONTROLLER_MAX 4

#define SQRT2	1.41421356237f
#define PI		3.14159265359f
#define DEG2RAD_CONSTANT	PI / 180.f
#define RAD2DEG_CONSTANT	180.f / PI

#define CLIP(X, A, B) ((X < A) ? A : ((X > B) ? B : X))
#define ABS(X) (X < 0 ? -X : X)

struct vector
{
	float X;
	float Y;
};

struct button_state
{
	bool IsDownLastState;
	uint32_t Duration;
	bool IsDown;
};

struct controller_state
{
	struct
	{
		float XLastState;
		float YLastState;
		uint32_t Duration;
		float X;
		float Y;
	};

	union
	{
		button_state Buttons[4];
		struct
		{
			button_state Up;
			button_state Down;
			button_state Left;
			button_state Right;
		};
	};
};

struct input_state
{
	controller_state Controllers[4];
};

struct baddie
{
	vector Position;
	float Radius;
	float Angle;
};

struct hero
{
	vector Position;
	int CurrentPathIndex;
	float DirectionFacing;
	float Radius;
	float HalfHeight;
};

struct scene
{
	hero Hero;
	baddie Baddies[255];
	int BaddieCount;
};

#define LOAD_GAME(name) void name(scene *Scene)
typedef LOAD_GAME(load_game);

#define UPDATE_AND_RENDER_GAME(name) void name( \
  input_state *Input, scene *Scene, float Dt  \
)
typedef UPDATE_AND_RENDER_GAME(update_and_render_game);
UPDATE_AND_RENDER_GAME(UpdateAndRenderGameStub) {

};

// Note(sigmasleep): Should be provided by platform layer.
// Note(sigmasleep): Is this the best place for this?
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
typedef DEBUG_DRAW_CIRCLE(debug_draw_semi_circle);

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

#define DEBUG_SET_COLOR(name) void name(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
typedef DEBUG_SET_COLOR(debug_set_color);