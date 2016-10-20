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

global_variable bool Running = true;

global_variable SDL_Point Clicks[255];
global_variable uint8_t ClicksSize;

global_variable SDL_Point Paths[255];
global_variable uint8_t PathSize;

SDL_Window *window;
SDL_Renderer *renderer;

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
	double X = X2 - X1;
	double Y = Y2 - Y1;

	return atan2(Y, X) * 180 / 3.14;
}

struct Coords
{
	double x, y;
};

struct CoordsQueue
{
	Coords Positions[255];
	uint8_t Size;
	uint8_t StartIndex;
};

internal bool
CoordsQueuePush(CoordsQueue *CQueue, Coords *C)
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

internal Coords
CoordsQueuePop(CoordsQueue *CQueue)
{
	Coords Position = CQueue->Positions[CQueue->StartIndex];
	CQueue->Size--;
	CQueue->StartIndex++;

	return Position;
}

internal Coords
CoordsQueuePeek(CoordsQueue *CQueue)
{
	return CQueue->Positions[CQueue->StartIndex];
}

internal Coords
CoordsQueuePeek2(CoordsQueue *CQueue)
{
	return CQueue->Positions[(uint8_t)(CQueue->StartIndex+1)];
}

internal void
CoordsQueueClear(CoordsQueue *CQueue)
{
	CQueue->Size = 0;
	CQueue->StartIndex = 0;
}

internal void
RenderCoordsQueue(CoordsQueue *CQueue)
{
	uint8_t StopIndex = CQueue->StartIndex + CQueue->Size - 1;
	for(uint8_t Index = CQueue->StartIndex; Index != StopIndex; Index++)
	{
		Coords Position1 = CQueue->Positions[Index];
		Coords Position2 = CQueue->Positions[(uint8_t)(Index + 1)];
		SDL_RenderDrawLine(renderer,
						   Position1.x, Position1.y,
						   Position2.x, Position2.y);
	}
}

struct Hero_
{
	Coords Position;
	CoordsQueue Waypoints;
	int CurrentPathIndex;
	double DirectionFacing;
	double Radius;
	double HalfHeight;
};

global_variable Hero_ Hero;

struct Baddie_
{
	Coords Position;
	double Radius;
	double Angle;
};

global_variable Baddie_ Baddie;

struct BaddieDebri
{
	Coords Position;
	double Radius;
	double Angle;
	double Segments;
	double TotalSegments;
};

internal void
NavigatePath(double Dt)
{
	if(Hero.Waypoints.Size > 1)
	{
		double Direction;
		Coords Position = CoordsQueuePeek2(&Hero.Waypoints);

		Direction = GetAngleBetweenPoints(
			Hero.Position.x, Hero.Position.y,
			Position.x, Position.y);

		double Distance = 50 * Dt;
		
		if(GetDistanceBetweenPoints(
			Hero.Position.x, Hero.Position.y,
			Position.x, Position.y) < 50 * Dt)
		{
			CoordsQueuePop(&Hero.Waypoints);
			Hero.Position.x = Position.x;
			Hero.Position.y = Position.y;
		}
		else
		{
			Hero.Position.x += cos(Direction * 3.14 / 180.f) * Distance;
			Hero.Position.y += sin(Direction * 3.14 / 180.f) * Distance;
		}

		Hero.DirectionFacing = Direction;
	}
}

internal void
BaddieMovement(double Dt)
{
	double Distance = 10 * Dt;

	Baddie.Position.x += cos(Baddie.Position.y/10) * Distance;
	Baddie.Position.y += sin(Baddie.Position.x/10) * Distance;
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

	SDL_RenderDrawLines(renderer, Points, 4);
}

internal void
DrawSemiCircle(
	double X, double Y,
	double Radius,
	double Segments, double TotalSegments,
	double Angle)
{
	CoordsQueue Points;
	Points.Size = 0;
	Points.StartIndex = 0;

	double RadAngle = Angle * 3.14 / 180;

	for(int Index = 0; Index < Segments; Index++)
	{
		Coords Point;
		Point.x = X + cos(RadAngle + Index / TotalSegments * 3.14 * 2) * Radius;
		Point.y = Y + sin(RadAngle + Index / TotalSegments * 3.14 * 2) * Radius;
		CoordsQueuePush(&Points, &Point);
	}
	Coords Point;
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
CollideWithBaddie()
{
	double Distance = GetDistanceBetweenPoints(
		Hero.Position.x, Hero.Position.y,
		Baddie.Position.x, Baddie.Position.y
	);

	if(Distance < Hero.Radius + Baddie.Radius)
	{
		double PushDistance = Baddie.Radius - (Distance - Hero.Radius);
		double Direction = GetAngleBetweenPoints(
			Hero.Position.x, Hero.Position.y,
			Baddie.Position.x, Baddie.Position.y);
		Baddie.Position.x += cos(Direction * 3.14 / 180.f) * PushDistance;
		Baddie.Position.y += sin(Direction * 3.14 / 180.f) * PushDistance;
	}

	double x = Hero.Position.x + cos(Hero.DirectionFacing * 3.14 / 180.f) * Hero.HalfHeight;
	double y = Hero.Position.y + sin(Hero.DirectionFacing * 3.14 / 180.f) * Hero.HalfHeight;

	DrawCircle(x, y, 10, 15);

	Distance = GetDistanceBetweenPoints(
		x, y,
		Baddie.Position.x, Baddie.Position.y
	);

	if(Distance < Baddie.Radius)
	{
		double Direction = GetAngleBetweenPoints(
			x, y,
			Baddie.Position.x, Baddie.Position.y
		);

		Baddie.Angle = Direction;
	}
}

int
main(int argc, char* args[])
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		return 1;
	}

	window = SDL_CreateWindow("SDL Test",
							  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
							  SCREEN_WIDTH, SCREEN_HEIGHT,
							  0);
	
	renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_Event e;
	uint32_t dt, lastTime = SDL_GetTicks();

	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
						SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	
	Hero.Position.x = 0;
	Hero.Position.y = 0;
	Hero.DirectionFacing = 0;
	Hero.CurrentPathIndex = 0;
	Hero.Radius = 7;
	Hero.HalfHeight = acos(30 * 3.14 / 180) * Hero.Radius * 2;
	CoordsQueueClear(&Hero.Waypoints);
	CoordsQueuePush(&Hero.Waypoints, &Hero.Position);

	Baddie.Position.x = SCREEN_WIDTH / 2;
	Baddie.Position.y = SCREEN_HEIGHT / 2;
	Baddie.Radius = 14;

	while(Running) {
		dt = SDL_GetTicks() - lastTime;
		lastTime = SDL_GetTicks();
		
		while(SDL_PollEvent(&e) != 0)
		{
			switch(e.type)
			{
				case SDL_QUIT:
				{
					Running = false;
				} break;
				case SDL_MOUSEBUTTONDOWN:
				{
					SDL_MouseButtonEvent Event = e.button;
					if(Event.button == SDL_BUTTON_LEFT)
					{
						Coords Position;
						Position.x = Event.x;
						Position.y = Event.y;
						CoordsQueuePush(&Hero.Waypoints, &Position);

					}
					else if(Event.button == SDL_BUTTON_RIGHT)
					{
						CoordsQueueClear(&Hero.Waypoints);
						CoordsQueuePush(&Hero.Waypoints, &Hero.Position);
					}
				} break;
			}
		}

		NavigatePath(dt/1000.f);
		BaddieMovement(dt / 1000.f);
		CollideWithBaddie();

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &fillRect);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		RenderCoordsQueue(&Hero.Waypoints);
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

		DrawTriangle(Hero.Position.x, Hero.Position.y,
					 Hero.DirectionFacing,
					 Hero.HalfHeight);
		DrawCircle(Hero.Position.x, Hero.Position.y, Hero.Radius, 32);
		DrawSemiCircle(Baddie.Position.x, Baddie.Position.y,
					   Baddie.Radius,
					   16, 32,
					   Baddie.Angle);
		DrawSemiCircle(Baddie.Position.x, Baddie.Position.y,
					   Baddie.Radius,
					   16, 32,
					   Baddie.Angle + 180);
		SDL_RenderPresent(renderer);

		SDL_Delay(1);
	}

	SDL_Quit();

	return(0);
}
