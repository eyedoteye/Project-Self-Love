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

// Note(sigmasleep): Should be provided by platform layer.
// Note(sigmasleep): Is this the best place for this?
internal void
DrawSemiCircle(
	float X, float Y,
	float Radius,
	int Segments, int TotalSegments,
	float Angle);

internal void
DrawCircle(float X, float Y, float Radius, int Segments);

internal void
DrawTriangle(float X, float Y, float Angle, float HalfHeight);

internal void
FillBox(float X, float Y, float Width, float Height);

internal void
DrawBox(float X, float Y, float Width, float Height);

internal void
DrawLine(float X1, float Y1, float X2, float Y2);

internal void
SetColor(uint8_t R, uint8_t G, uint8_t B, uint8_t A);
