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
GetAngleBetweenPoints(SDL_Point One, SDL_Point Two)
{
	int xOff = Two.x - One.x;
	int yOff = Two.y - One.y;

	return atan2(yOff, xOff) * 180 / 3.14;
}

struct Coords
{
	double x, y;
};

struct Hero_
{
	Coords Position;
	int CurrentPathIndex;
	double DirectionFacing;
	bool InPath;
};

global_variable Hero_ Hero;

internal void
RecordMouseClick(int x, int y)
{
	Clicks[ClicksSize].x = x;
	Clicks[ClicksSize].y = y;
	ClicksSize++;
	if(Hero.InPath == false)
	{
		Hero.Position.x = Clicks[0].x;
		Hero.Position.y = Clicks[0].y;
		Hero.CurrentPathIndex = 0;
	}
	if(ClicksSize >= 2)
	{
		Hero.InPath = true;
	}
}

internal void
ClearMouseClicks()
{
	ClicksSize = 1;
	Clicks[0].x = Hero.Position.x;
	Clicks[0].y = Hero.Position.y;
	Hero.InPath = false;
}

internal void
NavigatePath(double Dt)
{
	if(Hero.InPath && Hero.CurrentPathIndex+1 != ClicksSize)
	{
		double Direction;

		Direction = GetAngleBetweenPoints(Clicks[Hero.CurrentPathIndex],
										Clicks[Hero.CurrentPathIndex+1]);

		if(GetDistanceBetweenPoints(Hero.Position.x, Hero.Position.y,
									Clicks[Hero.CurrentPathIndex + 1].x,
									Clicks[Hero.CurrentPathIndex + 1].y)
									< 50 * Dt)
		{
			Hero.CurrentPathIndex++;
			Hero.Position.x = Clicks[Hero.CurrentPathIndex].x;
			Hero.Position.y = Clicks[Hero.CurrentPathIndex].y;
		}
		else
		{
			Hero.Position.x += cos(Direction * 3.14 / 180.f) * Dt * 50;
			Hero.Position.y += sin(Direction * 3.14 / 180.f) * Dt * 50;
		}
		Hero.DirectionFacing = Direction;
	}
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
	/*
	SDL_Surface* image = SDL_LoadBMP("image1.bmp");
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);*/

	SDL_Event e;
	uint32_t dt, lastTime = SDL_GetTicks();

	SDL_Rect fillRect = { SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
						SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	
	Hero.Position.x = 0;
	Hero.Position.y = 0;
	Hero.DirectionFacing = 0;
	Hero.CurrentPathIndex = 0;
	Hero.InPath = false;

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
						RecordMouseClick(Event.x, Event.y);
					}
					else if(Event.button == SDL_BUTTON_RIGHT)
					{
						ClearMouseClicks();
					}
				} break;
			}
		}

		/*char debugString[100];
		snprintf(debugString, 100, "debug\n");
		OutputDebugStringA(debugString);*/

		NavigatePath(dt/1000.f);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &fillRect);
		//SDL_Rect dimensions;
		//dimensions.x = 100;
		//dimensions.y = 50;
		//SDL_QueryTexture(texture, NULL, NULL, &dimensions.w, &dimensions.h);
		//dimensions.h *= 2;
		//SDL_RenderCopyEx(renderer, texture, NULL, &dimensions,
		//				 45.f, NULL, SDL_FLIP_NONE);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawLines(renderer, Clicks, ClicksSize);
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
		local_persist int Angle = 0;
		Angle += .6 * dt;
		DrawTriangle(Hero.Position.x, Hero.Position.y,
					 Hero.DirectionFacing,
					 15);
		SDL_RenderPresent(renderer);

		SDL_Delay(1);
	}

	//SDL_FreeSurface(image);

	SDL_Quit();

	return(0);
}
