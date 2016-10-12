#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include "SDL.h"

#define global_variable static
#define local_persist static

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 270

bool init();
bool loadMedia();
void close();

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gHelloWorld = NULL;

global_variable int JOYSTICK_DEAD_ZONE = 8000;
global_variable SDL_Joystick* gGameController;

bool 
init()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		OutputDebugStringA("init Error: ");
		OutputDebugStringA(SDL_GetError());
		return false;
	}

	gWindow = SDL_CreateWindow("Mak Gam",
							   SDL_WINDOWPOS_UNDEFINED,
							   SDL_WINDOWPOS_UNDEFINED,
							   SCREEN_WIDTH, SCREEN_HEIGHT,
							   SDL_WINDOW_SHOWN);
	
	if(gWindow == NULL)
	{
		printf("init Error: ");
		OutputDebugStringA(SDL_GetError());
		return false;
	}

	gScreenSurface = SDL_GetWindowSurface(gWindow);

	if(SDL_NumJoysticks() < 1)
	{
		OutputDebugStringA("no controller!");
	}
	else
	{
		gGameController = SDL_JoystickOpen(0);
		if(gGameController == NULL)
		{
			OutputDebugStringA("Controler error:");
			OutputDebugStringA(SDL_GetError());
		}
	}

	return true;
}

bool loadMedia()
{
	gHelloWorld = SDL_LoadBMP("image1.bmp");
	if(gHelloWorld == NULL)
	{
		OutputDebugStringA("loadMedia Error: ");
		OutputDebugStringA(SDL_GetError());
		return false;
	}
	return true;
}

void close()
{
	SDL_FreeSurface(gHelloWorld);
	gHelloWorld = NULL;

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_JoystickClose(gGameController);

	SDL_Quit();
}

int
main(int argc, char* args[])
{
	init();
	loadMedia();

	bool quit = false;
	SDL_Event e;

	SDL_Rect dstrect;
	dstrect.x = 0;
	dstrect.y = 0;
	dstrect.w = gHelloWorld->w;
	dstrect.h = gHelloWorld->h;

	int16_t xDir = 0;
	uint32_t dt, lastTime = SDL_GetTicks();
	while(!quit) {
		dt = SDL_GetTicks() - lastTime;
		lastTime = SDL_GetTicks();
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		

		dstrect.x += (int)(120 * dt / 1000.f) * SDL_JoystickGetAxis(gGameController, 0)/10000;
		dstrect.y += (int)(120 * dt / 1000.f) * SDL_JoystickGetAxis(gGameController, 1) / 10000;
		
		SDL_BlitSurface(gHelloWorld, NULL, gScreenSurface, &dstrect);
		SDL_UpdateWindowSurface(gWindow);

		SDL_Delay(10);

	}

	close();

	return(0);
}
