#include <stdio.h>
#include "SDL.h"

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 270;

int main(int argc, char* args[])
{
	SDL_Window* window = NULL;

	SDL_Surface* screenSurface = NULL;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Initilization Error: %s\n", SDL_GetError());
	}
	else
	{
		window = SDL_CreateWindow("SDL Test",
								  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
								  SCREEN_WIDTH, SCREEN_HEIGHT,
								  SDL_WINDOW_SHOWN);
		if(window == NULL)
		{
			printf("Window creation Error: %s\n", SDL_GetError());
		}
		else
		{
			screenSurface = SDL_GetWindowSurface(window);
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
			SDL_UpdateWindowSurface(window);
			SDL_Delay(1);
		}
	}

	SDL_DestroyWindow(window);
	SDL_Quit();

	return(0);
}
