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

global_variable bool GlobalRunning = true;

global_variable SDL_Window *GlobalWindow;
global_variable SDL_Renderer *GlobalRenderer;

global_variable double GlobalDt;

internal double
GetDistanceBetweenPoints(double X1, double Y1, double X2, double Y2)
{
	double X = X2 - X1;
	double Y = Y2 - Y1;
	return sqrt(X * X + Y * Y);
}

internal double
GetAngleBetweenPoints(double X1, double Y1, double X2, double Y2)
{
	double Y = Y2 - Y1;
	double X = X2 - X1;

	return atan2(Y, X) * 180 / 3.14;
}

struct coords
{
	double x, y;
};

struct coords_queue
{
	coords Positions[255];
	uint8_t Size;
	uint8_t StartIndex;
};

internal bool
CoordsQueuePush(coords_queue *CQueue, coords *C)
{ 
	if((uint8_t)(CQueue->Size + CQueue->StartIndex + 1) == CQueue->StartIndex)
	{
		return false;
	}

	CQueue->Positions[CQueue->StartIndex + CQueue->Size].x = C->x;
	CQueue->Positions[CQueue->StartIndex + CQueue->Size].y = C->y;
	CQueue->Size++;

	return true;
}

internal coords
CoordsQueuePop(coords_queue *CQueue)
{
	coords Position = CQueue->Positions[CQueue->StartIndex];
	CQueue->Size--;
	CQueue->StartIndex++;

	return Position;
}

internal coords
CoordsQueuePeek(coords_queue *CQueue)
{
	return CQueue->Positions[CQueue->StartIndex];
}

internal coords
CoordsQueuePeek2(coords_queue *CQueue)
{
	return CQueue->Positions[(uint8_t)(CQueue->StartIndex+1)];
}

internal void
CoordsQueueClear(coords_queue *CQueue)
{
	CQueue->Size = 0;
	CQueue->StartIndex = 0;
}

internal void
RenderCoordsQueue(coords_queue *CQueue)
{
	if(CQueue->Size > 0)
	{
		uint8_t StopIndex = CQueue->StartIndex + CQueue->Size - 1;
		for(uint8_t Index = CQueue->StartIndex; Index != StopIndex; Index++)
		{
			coords Position1 = CQueue->Positions[Index];
			coords Position2 = CQueue->Positions[(uint8_t)(Index + 1)];
			SDL_RenderDrawLine(GlobalRenderer,
							   Position1.x, Position1.y,
							   Position2.x, Position2.y);
		}
	}
}


struct button_state
{
	bool IsDownLastState;
	uint32_t Duration;
	bool IsDown;
};

// NOTE(sigmasleep): There needs to be a threshold for determining a change in direction.

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
	coords Position;
	double Radius;
	double Angle;
};

struct hero
{
	coords Position;
	coords_queue Waypoints;
	int CurrentPathIndex;
	double DirectionFacing;
	double Radius;
	double HalfHeight;
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

global_variable hero GlobalHero;

internal void
NavigatePath()
{
	if(GlobalHero.Waypoints.Size > 1)
	{
		double Direction;
		coords Position = CoordsQueuePeek2(&GlobalHero.Waypoints);

		Direction = GetAngleBetweenPoints(
			GlobalHero.Position.x, GlobalHero.Position.y,
			Position.x, Position.y);

		double Distance = 50 * GlobalDt;
		
		if(GetDistanceBetweenPoints(
			GlobalHero.Position.x, GlobalHero.Position.y,
			Position.x, Position.y) < Distance)
		{
			CoordsQueuePop(&GlobalHero.Waypoints);
			GlobalHero.Position.x = Position.x;
			GlobalHero.Position.y = Position.y;
		}
		else
		{
			GlobalHero.Position.x += cos(Direction * 3.14 / 180.f) * Distance;
			GlobalHero.Position.y += sin(Direction * 3.14 / 180.f) * Distance;
		}

		GlobalHero.DirectionFacing = Direction;
	}
}

internal void
BaddieMovement(baddie *Baddie)
{
	double Distance = 10 * GlobalDt;

	Baddie->Position.x += cos(Baddie->Position.y/10) * Distance;
	Baddie->Position.y += sin(Baddie->Position.x/10) * Distance;
}

internal void
DrawTriangle(int X, int Y, double Angle, int HalfHeight)
{
	SDL_Point Points[4];
	Points[0].x = X + cos(Angle * 3.14 / 180.f) * HalfHeight;
	Points[0].y = Y + sin(Angle * 3.14 / 180.f) * HalfHeight;
	Points[3].x = Points[0].x;
	Points[3].y = Points[0].y;

	Points[1].x = X + cos((Angle + 120) * 3.14 / 180.f) * HalfHeight;
	Points[1].y = Y + sin((Angle + 120) * 3.14 / 180.f) * HalfHeight;

	Points[2].x = X + cos((Angle - 120) * 3.14 / 180.f) * HalfHeight;
	Points[2].y = Y + sin((Angle - 120) * 3.14 / 180.f) * HalfHeight;

	SDL_RenderDrawLines(GlobalRenderer, Points, 4);
}

internal void
DrawSemiCircle(
	double X, double Y,
	double Radius,
	double Segments, double TotalSegments,
	double Angle)
{
	coords_queue Points;
	Points.Size = 0;
	Points.StartIndex = 0;

	double RadAngle = Angle * 3.14 / 180;

	for(int Index = 0; Index < Segments; Index++)
	{
		coords Point;
		Point.x = X + cos(RadAngle + Index / TotalSegments * 3.14 * 2) * Radius;
		Point.y = Y + sin(RadAngle + Index / TotalSegments * 3.14 * 2) * Radius;
		CoordsQueuePush(&Points, &Point);
	}
	coords Point;
	Point.x = X + cos(RadAngle) * Radius;
	Point.y = Y + sin(RadAngle) * Radius;
	CoordsQueuePush(&Points, &Point);

	RenderCoordsQueue(&Points);
}

internal void
DrawCircle(double X, double Y, double Radius, double Segments)
{
	DrawSemiCircle(X, Y, Radius, Segments, Segments, 0);
}

internal void
RenderBaddie(baddie *Baddie)
{
	DrawSemiCircle(Baddie->Position.x, Baddie->Position.y,
				   Baddie->Radius,
				   16, 32,
				   Baddie->Angle);
	DrawSemiCircle(Baddie->Position.x, Baddie->Position.y,
				   Baddie->Radius,
				   16, 32,
				   Baddie->Angle + 180);
}

internal void
RunOnBaddiesInScene(scene *Scene, void (*BaddieFunction)(baddie*))
{
	for(int BaddieIndex = 0; BaddieIndex < Scene->BaddieCount; BaddieIndex++)
	{
		BaddieFunction(&(Scene->Baddies[BaddieIndex]));
	}
}

internal void
RenderScene(scene *Scene)
{
	RunOnBaddiesInScene(Scene, RenderBaddie);
}

internal void
CollideWithBaddie(baddie *Baddie)
{
	double Distance = GetDistanceBetweenPoints(
		GlobalHero.Position.x, GlobalHero.Position.y,
		Baddie->Position.x, Baddie->Position.y
	);

	if(Distance < GlobalHero.Radius + Baddie->Radius)
	{
		double PushDistance = Baddie->Radius - (Distance - GlobalHero.Radius);
		double Direction = GetAngleBetweenPoints(
			GlobalHero.Position.x, GlobalHero.Position.y,
			Baddie->Position.x, Baddie->Position.y);
		Baddie->Position.x += cos(Direction * 3.14 / 180.f) * PushDistance;
		Baddie->Position.y += sin(Direction * 3.14 / 180.f) * PushDistance;
	}

	double x = GlobalHero.Position.x + cos(GlobalHero.DirectionFacing * 3.14 / 180.f) * GlobalHero.HalfHeight;
	double y = GlobalHero.Position.y + sin(GlobalHero.DirectionFacing * 3.14 / 180.f) * GlobalHero.HalfHeight;

	Distance = GetDistanceBetweenPoints(
		x, y,
		Baddie->Position.x, Baddie->Position.y
	);

	if(Distance < Baddie->Radius)
	{
		double Direction = GetAngleBetweenPoints(
			x, y,
			Baddie->Position.x, Baddie->Position.y
		);

		Baddie->Angle = Direction;
	}
}

struct vector
{
	float x;
	float y;
};

#define CLIP(X, A, B) ((X < A) ? A : ((X > B) ? B : X))
#define SQRT2 1.41421356237
#define ABS(X) (X < 0 ? -X : X)

internal void
ProcessControllerMovement(controller_state *Controller, vector *Movement)
{
	float x = 0;
	float y = 0;

	if(Controller->Left.IsDown)
	{
		x -= 1;
	}
	if(Controller->Right.IsDown)
	{
		x += 1;
	}
	if(Controller->Up.IsDown)
	{
		y -= 1;
	}
	if(Controller->Down.IsDown)
	{
		y += 1;
	}

	if(x != 0 && y != 0)
	{
		x /= SQRT2;
		y /= SQRT2;
	}

	x += Controller->X;
	y += Controller->Y;

	Movement->x = CLIP(x, -1.f, 1.f);
	Movement->y = CLIP(y, -1.f, 1.f);
}

internal void
MovePlayer(hero *Hero, input_state *Input)
{
	vector InputMovement;

	ProcessControllerMovement(&Input->Controllers[0], &InputMovement);

	Hero->Position.x += 100 * InputMovement.x * GlobalDt;
	Hero->Position.y += 100 * InputMovement.y * GlobalDt;
}

// Note(sigmasleep): This should not have any calls to SDL in it
internal void
RenderGame(input_state *Input, scene *Scene)
{
	//NavigatePath();
	MovePlayer(&Scene->Hero, Input);
	RunOnBaddiesInScene(Scene, BaddieMovement);
	//RunOnBaddiesInScene(Scene, CollideWithBaddie);

	RenderScene(Scene);
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

	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
						SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	
	
	CoordsQueueClear(&GlobalHero.Waypoints);
	CoordsQueuePush(&GlobalHero.Waypoints, &GlobalHero.Position);

	baddie Baddie = {};

	Baddie.Position.x = SCREEN_WIDTH / 2;
	Baddie.Position.y = SCREEN_HEIGHT / 2;
	Baddie.Radius = 14;

	scene Scene = {};

	AddBaddieToScene(&Baddie, &Scene);
	Baddie.Position.x += SCREEN_WIDTH / 4;
	AddBaddieToScene(&Baddie, &Scene);

	Scene.Hero.Position.x = SCREEN_WIDTH / 4;
	Scene.Hero.Position.y = SCREEN_HEIGHT / 4;
	Scene.Hero.DirectionFacing = 0;
	Scene.Hero.CurrentPathIndex = 0;
	Scene.Hero.Radius = 7;
	Scene.Hero.HalfHeight = acos(30 * 3.14 / 180) * Scene.Hero.Radius * 2;

	input_state Input = {};

	SDL_GameController *Controller1 = SDL_GameControllerOpen(0);
	while(GlobalRunning) {
		GlobalDt = (SDL_GetTicks() - lastTime)/1000.f;
		lastTime = SDL_GetTicks();
		
		while(SDL_PollEvent(&e) != 0)
		{
			switch(e.type)
			{
				case SDL_QUIT:
				{
					GlobalRunning = false;
				} break;
				case SDL_MOUSEBUTTONDOWN:
				{
					SDL_MouseButtonEvent Event = e.button;
					if(Event.button == SDL_BUTTON_LEFT)
					{
						coords Position;
						Position.x = Event.x;
						Position.y = Event.y;
						CoordsQueuePush(&GlobalHero.Waypoints, &Position);

					}
					else if(Event.button == SDL_BUTTON_RIGHT)
					{
						CoordsQueueClear(&GlobalHero.Waypoints);
						CoordsQueuePush(&GlobalHero.Waypoints, &GlobalHero.Position);
					}
				} break;
				case SDL_CONTROLLERAXISMOTION:
				{
					SDL_ControllerAxisEvent Event = e.caxis;

					// Note(sigmasleep): Normalization via division by int16 min/max values
					float Value = Event.value < 0 ? Event.value / 32768.f : Event.value / 32767.f;

					// Todo(sigmasleep): Move deadzone to a better place
					float Deadzone = 0.2;
					Value = ABS(Value) > Deadzone ? Value : 0;

					if(Event.which < 4)
					{
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

					if(Event.which < 4)
					{
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
			}
		}

		SDL_SetRenderDrawColor(GlobalRenderer, 255, 0, 0, 255);
		SDL_RenderClear(GlobalRenderer);
		SDL_SetRenderDrawColor(GlobalRenderer, 0, 255, 0, 255);
		SDL_RenderFillRect(GlobalRenderer, &fillRect);

		SDL_SetRenderDrawColor(GlobalRenderer, 255, 255, 255, 255);
		RenderCoordsQueue(&GlobalHero.Waypoints);

		SDL_SetRenderDrawColor(GlobalRenderer, 0, 0, 255, 255);
		RenderGame(&Input, &Scene);

		DrawTriangle(Scene.Hero.Position.x, Scene.Hero.Position.y,
					 Scene.Hero.DirectionFacing,
					 Scene.Hero.HalfHeight);
		DrawCircle(Scene.Hero.Position.x, Scene.Hero.Position.y, Scene.Hero.Radius, 32);
		
		SDL_RenderPresent(GlobalRenderer);

		SDL_Delay(1);
	}

	SDL_GameControllerClose(Controller1);

	SDL_Quit();

	return(0);
}
