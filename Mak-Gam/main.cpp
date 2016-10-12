#include <stdio.h>
#include <stdbool.h>
#include <windows.h>
#include <stdint.h>
#include "SDL.h"

#define global_variable static
#define local_persist static

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270

global_variable bool Running = true;

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
	
	SDL_Event e;
	uint32_t dt, lastTime = SDL_GetTicks();
	
	while(Running) {
		dt = SDL_GetTicks() - lastTime;
		lastTime = SDL_GetTicks();
		
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
			{
				Running = false;
			}
		}

		char debugString[100];
		snprintf(debugString, 100, "debug\n");
		OutputDebugStringA(debugString);

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);

		SDL_Delay(1);
	}

	SDL_Quit();

	return(0);
}
