#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stdint.h>
#include "SDL.h"

#define internal static
#define global_variable static
#define local_persist static

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270

global_variable bool Running = true;

global_variable SDL_Point Clicks[255];
global_variable uint8_t ClicksSize;

internal void
RecordMouseClick(int x, int y)
{
	Clicks[ClicksSize].x = x;
	Clicks[ClicksSize++].y = y;
}

internal void
ClearMouseClicks()
{
	ClicksSize = 0;
}

//draw line

int
main(int argc, char* args[])
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	
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
		SDL_RenderPresent(renderer);

		SDL_Delay(1);
	}

	//SDL_FreeSurface(image);

	SDL_Quit();

	return(0);
}
