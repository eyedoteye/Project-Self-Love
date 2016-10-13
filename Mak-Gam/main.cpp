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

SDL_Window *window;
SDL_Renderer *renderer;

internal void
RecordMouseClick(int x, int y)
{
	Clicks[ClicksSize].x = x;
	Clicks[ClicksSize].y = y;
	ClicksSize++;
}

internal void
ClearMouseClicks()
{
	ClicksSize = 0;
}

internal double
GetAngleBetweenPoints(SDL_Point One, SDL_Point Two)
{
	int xOff = Two.x - One.x;
	int yOff = Two.y - One.y;

	return atan2(yOff, xOff) * 180.f / 3.14;
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
		DrawTriangle(Clicks[0].x, Clicks[0].y,
					 GetAngleBetweenPoints(Clicks[0],Clicks[1]),
					 15);
		SDL_RenderPresent(renderer);

		SDL_Delay(1);
	}

	//SDL_FreeSurface(image);

	SDL_Quit();

	return(0);
}
