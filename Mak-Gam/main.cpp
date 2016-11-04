#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stdint.h>
#include <math.h>
#include "SDL.h"

#define internal static
#define global_variable static
#define local_persist static

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270

#define CLIP(X, A, B) ((X < A) ? A : ((X > B) ? B : X))
#define SQRT2 1.41421356237f
#define ABS(X) (X < 0 ? -X : X)

global_variable bool GlobalRunning = true;

global_variable SDL_Window *GlobalWindow;
global_variable SDL_Renderer *GlobalRenderer;

global_variable float GlobalDt;

struct vector
{
	float X;
	float Y;
};

internal float
GetDistanceBetweenPoints(float X1, float Y1, float X2, float Y2)
{
	float X = X2 - X1;
	float Y = Y2 - Y1;
	return (float)sqrt(X * X + Y * Y);
}

internal float
GetAngleBetweenPoints(float X1, float Y1, float X2, float Y2)
{
	float Y = Y2 - Y1;
	float X = X2 - X1;

	return (float)(atan2(Y, X) * 180 / 3.14);
}

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

internal void
AddBaddieToScene(baddie *Baddie, scene *Scene)
{
	Scene->Baddies[Scene->BaddieCount].Angle = Baddie->Angle;
	Scene->Baddies[Scene->BaddieCount].Position = Baddie->Position;
	Scene->Baddies[Scene->BaddieCount].Radius = Baddie->Radius;
	Scene->BaddieCount++;
}

internal void
BaddieMovement(baddie *Baddie)
{
	float Distance = 10 * GlobalDt;

	Baddie->Position.X += (float)cos(Baddie->Position.Y / 10) * Distance;
	Baddie->Position.Y += (float)sin(Baddie->Position.X / 10) * Distance;
}

internal void
DrawTriangle(int X, int Y, float Angle, int HalfHeight)
{
	SDL_Point Points[4];
	Points[0].x = (int)(X + cos(Angle * 3.14 / 180.f) * HalfHeight);
	Points[0].y = (int)(Y + sin(Angle * 3.14 / 180.f) * HalfHeight);
	Points[3].x = Points[0].x;
	Points[3].y = Points[0].y;

	Points[1].x = (int)(X + cos((Angle + 120) * 3.14 / 180.f) * HalfHeight);
	Points[1].y = (int)(Y + sin((Angle + 120) * 3.14 / 180.f) * HalfHeight);

	Points[2].x = (int)(X + cos((Angle - 120) * 3.14 / 180.f) * HalfHeight);
	Points[2].y = (int)(Y + sin((Angle - 120) * 3.14 / 180.f) * HalfHeight);

	SDL_RenderDrawLines(GlobalRenderer, Points, 4);
}


internal void
DrawSemiCircle(
	float X, float Y,
	float Radius,
	float Segments, float TotalSegments,
	float Angle)
{
	float RadAngle = Angle * 3.14f / 180;

	vector Position1;
	vector Position2;
	Position1.X = X + (float)cos(RadAngle) * Radius;
	Position1.Y = Y + (float)sin(RadAngle) * Radius;

	for(int PointNum = 0; PointNum < Segments; PointNum++)
	{
		Position2.X = X + (float)cos(RadAngle + PointNum / TotalSegments * 3.14 * 2) * Radius;
		Position2.Y = Y + (float)sin(RadAngle + PointNum / TotalSegments * 3.14 * 2) * Radius;
		SDL_RenderDrawLine(GlobalRenderer,
						   (int)Position1.X, (int)Position1.Y,
						   (int)Position2.X, (int)Position2.Y);
		Position1.X = Position2.X;
		Position1.Y = Position2.Y;
	}

	Position2.X = X + (float)cos(RadAngle) * Radius;
	Position2.Y = Y + (float)sin(RadAngle) * Radius;
	SDL_RenderDrawLine(GlobalRenderer,
					   (int)Position1.X, (int)Position1.Y,
					   (int)Position2.X, (int)Position2.Y);
}

internal void
DrawCircle(float X, float Y, float Radius, float Segments)
{
	DrawSemiCircle(X, Y, Radius, Segments, Segments, 0);
}

internal void
DrawBox(float X, float Y, float Width, float Height)
{
	SDL_Rect FillRect;
	FillRect.x = (int)X;
	FillRect.y = (int)Y;
	FillRect.w = (int)Width;
	FillRect.h = (int)Height;

	SDL_RenderDrawRect(GlobalRenderer, &FillRect);
}

internal void
RenderBaddie(baddie *Baddie)
{
	DrawSemiCircle(Baddie->Position.X, Baddie->Position.Y,
				   Baddie->Radius,
				   16, 32,
				   Baddie->Angle);
	DrawSemiCircle(Baddie->Position.X, Baddie->Position.Y,
				   Baddie->Radius,
				   16, 32,
				   Baddie->Angle + 180);
	DrawBox(Baddie->Position.X - Baddie->Radius, Baddie->Position.Y - Baddie->Radius,
			Baddie->Radius * 2, Baddie->Radius * 2);
}

internal void
RunOnBaddiesInScene(scene *Scene, void(*BaddieFunction)(baddie*))
{
	for(int BaddieIndex = 0; BaddieIndex < Scene->BaddieCount; BaddieIndex++)
	{
		BaddieFunction(&(Scene->Baddies[BaddieIndex]));
	}
}

internal void
RenderScene(scene *Scene)
{
	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

	SDL_SetRenderDrawColor(GlobalRenderer, 255, 0, 0, 255);
	SDL_RenderClear(GlobalRenderer);
	SDL_SetRenderDrawColor(GlobalRenderer, 0, 255, 0, 255);
	SDL_RenderFillRect(GlobalRenderer, &fillRect);

	SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 255, 255);
	RunOnBaddiesInScene(Scene, RenderBaddie);

	DrawTriangle((int)Scene->Hero.Position.X, (int)Scene->Hero.Position.Y,
				 Scene->Hero.DirectionFacing,
				 (int)Scene->Hero.HalfHeight);
	DrawCircle(Scene->Hero.Position.X, Scene->Hero.Position.Y, Scene->Hero.Radius, 32);
	//DrawBox(Scene->Hero.Position.X - Scene->Hero.Radius, Scene->Hero.Position.Y - Scene->Hero.Radius,
	//		Scene->Hero.Radius*2, Scene->Hero.Radius*2);
}

internal bool
FillCollisionVectorCircleToCircle(
	vector *CollisionVector,
	float X1, float Y1, float R1,
	float X2, float Y2, float R2
)
{
	float XDistance = X2 - X1;
	float YDistance = Y2 - Y1;

	float RTotal = R1 + R2;
	float DistanceSquared = XDistance * XDistance + YDistance * YDistance;

	if(DistanceSquared < RTotal * RTotal)
	{
		float Distance = (float)sqrt(DistanceSquared);
		float UnitXDistance = XDistance / Distance;
		float UnitYDistance = YDistance / Distance;
		float CollisionDistance = RTotal - Distance;
		CollisionVector->X = UnitXDistance * CollisionDistance;
		CollisionVector->Y = UnitYDistance * CollisionDistance;

		return true;
	}

	return false;
}

internal bool
FillCollisionVectorLineToCircle(
	vector *CollisionVector,
	float X1, float Y1, float X2, float Y2,
	float X3, float Y3, float R3
)
{
	vector Hypotenuse;
	Hypotenuse.X = X3 - X1;
	Hypotenuse.Y = Y3 - Y1;

	vector Line;
	Line.X = X2 - X1;
	Line.Y = Y2 - Y1;

	float DotProduct = Hypotenuse.X * Line.X + Hypotenuse.Y * Line.Y;

	float LineLengthSquared = Line.X * Line.X + Line.Y * Line.Y;

	float ClippedDotProduct = CLIP(DotProduct, 0, LineLengthSquared);

	float ProjectionConstant = (ClippedDotProduct / LineLengthSquared);

	vector HypotenuseProjection;
	HypotenuseProjection.X = ProjectionConstant * Line.X;
	HypotenuseProjection.Y = ProjectionConstant * Line.Y;

		
	/*char output[255];
	snprintf(output, 255, "Line: %f %f\n",
			 Line.X, Line.Y);
	OutputDebugStringA(output);
	snprintf(output, 255, "DotProduct: %f %f\n", 
			 DotProduct);
	OutputDebugStringA(output);
	snprintf(output, 255, "LineLengthSquared: %f\n", 
			 LineLengthSquared);
	OutputDebugStringA(output);
	snprintf(output, 255, "ProjectionConstant: %f\n",
			 ProjectionConstant);
	OutputDebugStringA(output);
	snprintf(output, 255, "HypotenuseProjection: %f %f\n",
			 HypotenuseProjection.X, HypotenuseProjection.Y);
	OutputDebugStringA(output);*/

	return FillCollisionVectorCircleToCircle(
		CollisionVector,
		HypotenuseProjection.X, HypotenuseProjection.Y, 0,
		Hypotenuse.X, Hypotenuse.Y, R3
	);
}

/*internal bool
FillCollisionVectorLineToLine(
	vector *CollisionVector,
	float X1, float Y1, float X2, float Y2,
	float X3, float Y3, float X4, float Y4
)
{
	return false;
}*/

internal void
CollideWithBaddie(hero *Hero, baddie *Baddie)
{
	vector CollisionVector;

	if(FillCollisionVectorCircleToCircle(&CollisionVector,
										 Hero->Position.X, Hero->Position.Y, Hero->Radius,
										 Baddie->Position.X, Baddie->Position.Y, Baddie->Radius))
	{
		Baddie->Position.X += CollisionVector.X;
		Baddie->Position.Y += CollisionVector.Y;
	}

	float X = Hero->Position.X + (float)cos(Hero->DirectionFacing * 3.14 / 180.f) * Hero->HalfHeight;
	float Y = Hero->Position.Y + (float)sin(Hero->DirectionFacing * 3.14 / 180.f) * Hero->HalfHeight;

	float Distance = GetDistanceBetweenPoints(
		X, Y,
		Baddie->Position.X, Baddie->Position.Y
	);

	if(Distance < Baddie->Radius)
	{
		float Direction = GetAngleBetweenPoints(
			X, Y,
			Baddie->Position.X, Baddie->Position.Y
		);

		Baddie->Angle = Direction;
	}
}

internal void
ProcessControllerMovement(controller_state *Controller, vector *Movement)
{
	float X = 0;
	float Y = 0;

	if(Controller->Left.IsDown)
	{
		X -= 1;
	}
	if(Controller->Right.IsDown)
	{
		X += 1;
	}
	if(Controller->Up.IsDown)
	{
		Y -= 1;
	}
	if(Controller->Down.IsDown)
	{
		Y += 1;
	}

	if(X != 0 && Y != 0)
	{
		X /= SQRT2;
		Y /= SQRT2;
	}

	X += Controller->X;
	Y += Controller->Y;

	Movement->X = CLIP(X, -1.f, 1.f);
	Movement->Y = CLIP(Y, -1.f, 1.f);
}

internal void
MovePlayer(hero *Hero, input_state *Input)
{
	vector InputMovement;

	ProcessControllerMovement(&Input->Controllers[0], &InputMovement);

	Hero->Position.X += 100 * InputMovement.X * GlobalDt;
	Hero->Position.Y += 100 * InputMovement.Y * GlobalDt;

	if(InputMovement.Y != 0 || InputMovement.X != 0)
		Hero->DirectionFacing = (float)atan2(InputMovement.Y, InputMovement.X) * 180 / 3.14f;
}

// Note(sigmasleep): This should not have any calls to SDL in it
internal void
RenderGame(input_state *Input, scene *Scene)
{
	MovePlayer(&Scene->Hero, Input);
	RunOnBaddiesInScene(Scene, BaddieMovement);
	for(int BaddieIndex = 0; BaddieIndex < Scene->BaddieCount; BaddieIndex++)
	{
		CollideWithBaddie(&Scene->Hero, &Scene->Baddies[BaddieIndex]);
	}

	vector RandomPoint1;
	vector RandomPoint2;
	RandomPoint1.X = 100;
	RandomPoint1.Y = 200;
	RandomPoint2.X = 300;
	RandomPoint2.Y = 100;

	vector CollisionVector;

	if(FillCollisionVectorLineToCircle(
		&CollisionVector,
		RandomPoint1.X, RandomPoint1.Y, RandomPoint2.X, RandomPoint2.Y,
		Scene->Hero.Position.X, Scene->Hero.Position.Y, Scene->Hero.Radius
	))
	{
		Scene->Hero.Position.X += CollisionVector.X;
		Scene->Hero.Position.Y += CollisionVector.Y;
	}

	RenderScene(Scene);

	SDL_RenderDrawLine(GlobalRenderer,
					   (int)RandomPoint1.X, (int)RandomPoint1.Y,
					   (int)RandomPoint2.X, (int)RandomPoint2.Y);
}

int
main(int argc, char* args[])
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		return 1;
	}

	GlobalWindow = SDL_CreateWindow("SDL Test",
									SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
									SCREEN_WIDTH, SCREEN_HEIGHT,
									0);

	GlobalRenderer = SDL_CreateRenderer(GlobalWindow, -1, 0);

	SDL_Event e;
	uint32_t lastTime = SDL_GetTicks();

	baddie Baddie = {};

	Baddie.Position.X = SCREEN_WIDTH / 2;
	Baddie.Position.Y = SCREEN_HEIGHT / 2;
	Baddie.Radius = 14;

	scene Scene = {};

	AddBaddieToScene(&Baddie, &Scene);
	Baddie.Position.X += SCREEN_WIDTH / 4;
	AddBaddieToScene(&Baddie, &Scene);

	Scene.Hero.Position.X = SCREEN_WIDTH / 4;
	Scene.Hero.Position.Y = SCREEN_HEIGHT / 4;
	Scene.Hero.DirectionFacing = 0;
	Scene.Hero.CurrentPathIndex = 0;
	Scene.Hero.Radius = 7;
	Scene.Hero.HalfHeight = (float)acos(30 * 3.14 / 180) * Scene.Hero.Radius * 2;

	input_state Input = {};

	SDL_GameController *Controller1 = SDL_GameControllerOpen(0);
	while(GlobalRunning) {
		GlobalDt = (SDL_GetTicks() - lastTime) / 1000.f;
		lastTime = SDL_GetTicks();

		while(SDL_PollEvent(&e) != 0)
		{
			switch(e.type)
			{
				case SDL_QUIT:
				{
					GlobalRunning = false;
				} break;
				case SDL_CONTROLLERAXISMOTION:
				{
					SDL_ControllerAxisEvent Event = e.caxis;

					// Note(sigmasleep): Normalization via division by int16 min/max values
					float Value = Event.value < 0 ? Event.value / 32768.f : Event.value / 32767.f;

					// Todo(sigmasleep): Move deadzone to a better place
					float Deadzone = 0.2f;
					Value = ABS(Value) > Deadzone ? Value : 0;

					// Todo(sigmasleep): Replace magic number with controllercount variable
					if(Event.which < 4)
					{
						// Todo(sigmasleep): Add timing
						controller_state* Controller = &Input.Controllers[Event.which];

						switch(Event.axis)
						{
							case SDL_CONTROLLER_AXIS_LEFTX:
							{
								Controller->XLastState = Controller->X;

								Controller->X = Value;
							} break;
							case SDL_CONTROLLER_AXIS_LEFTY:
							{
								Controller->YLastState = Controller->Y;

								Controller->Y = Value;
							} break;
						}
					}
				} break;
				case SDL_CONTROLLERBUTTONDOWN:
				case SDL_CONTROLLERBUTTONUP:
				{
					SDL_ControllerButtonEvent Event = e.cbutton;

					bool IsDown = Event.state == SDL_PRESSED;


					// Todo(sigmasleep): Replace magic number with controllercount variable
					if(Event.which < 4)
					{
						// Todo(sigmasleep): Add timing
						controller_state* Controller = &Input.Controllers[Event.which];
						switch(Event.button)
						{
							case SDL_CONTROLLER_BUTTON_DPAD_UP:
							{
								Controller->Up.IsDownLastState = Controller->Up.IsDown;
								Controller->Up.IsDown = IsDown;
							} break;
							case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							{
								Controller->Down.IsDownLastState = Controller->Down.IsDown;
								Controller->Down.IsDown = IsDown;
							} break;
							case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							{
								Controller->Left.IsDownLastState = Controller->Left.IsDown;
								Controller->Left.IsDown = IsDown;
							} break;
							case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							{
								Controller->Right.IsDownLastState = Controller->Right.IsDown;
								Controller->Right.IsDown = IsDown;
							} break;
						}
					}
				} break;
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				{
					SDL_KeyboardEvent Event = e.key;

					bool IsDown = Event.state == SDL_PRESSED;
					
					// Todo(sigmasleep): Add a way to change which player the keyboard controls
					// Todo(sigmasleep): Add timing
					controller_state* Controller = &Input.Controllers[0];
					switch(Event.keysym.scancode)
					{
						case SDL_SCANCODE_UP:
						{
							Controller->Up.IsDownLastState = Controller->Up.IsDown;
							Controller->Up.IsDown = IsDown;
						} break;
						case SDL_SCANCODE_DOWN:
						{
							Controller->Down.IsDownLastState = Controller->Down.IsDown;
							Controller->Down.IsDown = IsDown;
						} break;
						case SDL_SCANCODE_LEFT:
						{
							Controller->Left.IsDownLastState = Controller->Left.IsDown;
							Controller->Left.IsDown = IsDown;
						} break;
						case SDL_SCANCODE_RIGHT:
						{
							Controller->Right.IsDownLastState = Controller->Right.IsDown;
							Controller->Right.IsDown = IsDown;
						} break;
					}
				} break;
			}
		}

		RenderGame(&Input, &Scene);

		SDL_RenderPresent(GlobalRenderer);

		SDL_Delay(1);
	}

	SDL_GameControllerClose(Controller1);

	SDL_Quit();

	return(0);
}
