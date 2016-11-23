#pragma once

#include "common.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270
#define CONTROLLER_MAX 4

struct vector
{
	float X;
	float Y;
};

// Note (sigmasleep): WasReleasedSinceLastAction is set true by game layer,
// and false by api layer.
// WasReleasedSinceLastAction should be set to TRUE at game start.
struct button_state
{
  uint32_t Duration;
	bool WasReleasedSinceLastAction;
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
#define BUTTONCOUNT 6
		button_state Buttons[BUTTONCOUNT];
		struct
		{
			button_state Up;
			button_state Down;
			button_state Left;
			button_state Right;
      button_state RightBumper;
      button_state LeftBumper;
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

enum entity_type {
  HERO, BADDIE, DAGGER
};

enum dagger_state {
  FIRED, STUCK, RETURNING, RESTING
};

enum battle_choice {
  WARPTO, PULLBACK, NONE, PUSH
};

struct dagger
{
  vector Position;
  vector Velocity;
  float Radius;

  baddie *BaddieStuckTo;

  dagger_state State;
  battle_choice LastBattleChoice;
};

struct hero
{
	vector Position;
  vector Velocity;
  int CurrentPathIndex;
	float DirectionFacing;
	float Radius;
	float HalfHeight;
  bool RightBumperNotReleased;
  bool LeftBumperNotReleased;
   
  dagger Dagger;
};

struct scene
{
	hero Hero;
	baddie Baddies[255];
	int BaddieCount;
};

enum game_state
{
  INGAME, BATTLESCREEN
};

#define RESOLVE_COLLISION(name) void name(void *_Memory, void *This, void *Other, entity_type OtherType, vector *CollisionVector)
typedef RESOLVE_COLLISION(resolve_collision);

struct collision
{
  resolve_collision *Resolver;

  void* This;
  void* Other;
  entity_type OtherType;

  vector CollisionVector;
};

struct game_memory
{
  game_state GameState;
  scene *Scene;
  input_state *Input;

  collision Collisions[100];
  int CollisionsSize;

  float Dt;
  float TimeSpeed;
  float BattleScreenTimer;
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

#define DEBUG_SET_COLOR(name) void name(int R, int G, int B, int A)
typedef DEBUG_SET_COLOR(debug_set_color);

struct debug_tools
{
  debug_draw_semi_circle *DrawSemiCircle;
  debug_draw_circle *DrawCircle;
  debug_draw_triangle *DrawTriangle;
  debug_fill_box *FillBox;
  debug_draw_box *DrawBox;
  debug_draw_line *DrawLine;
  debug_set_color *SetColor;
};
global_variable debug_tools *GlobalDebugTools;

#define LOAD_GAME(name) void name(game_memory *Memory, debug_tools *DebugTools)
typedef LOAD_GAME(load_game);

#define RELOAD_GAME(name) void name(game_memory *Memory, debug_tools *DebugTools)
typedef RELOAD_GAME(reload_game);

#define UPDATE_AND_RENDER_GAME(name) void name(game_memory *Memory, float Dt)
typedef UPDATE_AND_RENDER_GAME(update_and_render_game);