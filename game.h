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
    int RawX;
    int RawY;
    float X;
		float Y;
    bool WasMovedThisFrame;
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
  NONE, WARPTO, PULLBACK, PUSH
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

#define RESOLVE_COLLISION(name) void name( \
  void *_Memory,                           \
  void *This,                              \
  void *Other, entity_type OtherType,      \
  vector *CollisionVector)
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

// Note: Should be provided by platform layer.
// Note: Is this the best place for these Debug Functions?
#define DEBUG_PRINT(name) void name( \
	char* OutputString,                \
	...                                \
)
typedef DEBUG_PRINT(debug_print);

struct debug_tools
{
	debug_print *Print;
};
global_variable debug_tools *GlobalDebugTools;

// Todo: Move this out of game api?
struct memory
{
  int Size;
  void *AllocatedSpace;
};

#define LOAD_GAME(name) void name(memory *Memory, debug_tools *DebugTools)
typedef LOAD_GAME(load_game);

#define RELOAD_GAME(name) void name(memory *Memory, debug_tools *DebugTools)
typedef RELOAD_GAME(reload_game);

#define UPDATE_GAME(name) void name(memory *Memory, float Dt)
typedef UPDATE_GAME(update_game);
