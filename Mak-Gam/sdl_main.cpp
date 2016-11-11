// Todo(sigmasleep): just create my own string copy, this is stupid
#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

#define internal static
#define global_variable static
#define local_persist static

#include "game.h"

#include <windows.h>
#include <stdio.h>
#include "SDL.h"

global_variable SDL_Renderer *GlobalRenderer;
global_variable bool GlobalRunning = true;
global_variable SDL_Window *GlobalWindow;

// Note(sigmasleep): Should these functions exist in platform layer?
DEBUG_SET_COLOR(DebugSetColor)
{
	SDL_SetRenderDrawColor(GlobalRenderer, (Uint8)R, (Uint8)G, (Uint8)B, (Uint8)A);
}

DEBUG_DRAW_LINE(DebugDrawLine)
{
  SDL_RenderDrawLine(GlobalRenderer,
    (int)X1, (int)Y1,
    (int)X2, (int)Y2);
}

DEBUG_DRAW_SEMI_CIRCLE(DebugDrawSemiCircle)
{
	float RadAngle = Angle * DEG2RAD_CONSTANT;

	vector Position1;
	vector Position2;
	Position1.X = X + cosf(RadAngle) * Radius;
	Position1.Y = Y + sinf(RadAngle) * Radius;

	for(int PointNum = 0; PointNum < Segments; PointNum++)
	{
		Position2.X = X + cosf(RadAngle + PointNum / (float)TotalSegments * PI * 2) * Radius;
		Position2.Y = Y + sinf(RadAngle + PointNum / (float)TotalSegments * PI * 2) * Radius;
		SDL_RenderDrawLine(
      GlobalRenderer,
			(int)Position1.X, (int)Position1.Y,
			(int)Position2.X, (int)Position2.Y);
		Position1.X = Position2.X;
		Position1.Y = Position2.Y;
	}

	Position2.X = X + cosf(RadAngle) * Radius;
	Position2.Y = Y + sinf(RadAngle) * Radius;
	SDL_RenderDrawLine(GlobalRenderer,
		(int)Position1.X, (int)Position1.Y,
		(int)Position2.X, (int)Position2.Y);
}

DEBUG_DRAW_CIRCLE(DebugDrawCircle)
{
	DebugDrawSemiCircle(X, Y, Radius, Segments, Segments, 0);
}

DEBUG_DRAW_TRIANGLE(DebugDrawTriangle)
{
	DebugDrawSemiCircle(X, Y, HalfHeight, 3, 3, Angle);
}

DEBUG_DRAW_BOX(DebugDrawBox)
{
	SDL_Rect DrawRect;
	DrawRect.x = (int)X;
	DrawRect.y = (int)Y;
	DrawRect.w = (int)Width;
	DrawRect.h = (int)Height;

	SDL_RenderDrawRect(GlobalRenderer, &DrawRect);
}

DEBUG_FILL_BOX(DebugFillBox)
{
	SDL_Rect FillRect;
	FillRect.x = (int)X;
	FillRect.y = (int)Y;
	FillRect.w = (int)Width;
	FillRect.h = (int)Height;

	SDL_RenderFillRect(GlobalRenderer, &FillRect);
}

typedef void* game_library;
struct game_functions
{
  time_t Timestamp;
  game_library Library;

  load_game *LoadGame;
  update_and_render_game *UpdateAndRenderGame;
};

internal int
GetTerminatedStringLength(char* String)
{
  int Length = 0;
  while (*String != '\0')
  {
    Length++;
    String++;
  }

  return Length;
}
internal void
LoadGameFunctions(game_functions *GameFunctions)
{
  char* BasePath;
  BasePath = SDL_GetBasePath();
  int BasePathLength = GetTerminatedStringLength(BasePath);
  
  char FilePath[200];
  strncpy(
    FilePath,
    BasePath, BasePathLength);
  strncpy(FilePath + BasePathLength,
    "MakGamGameLayer.dll", sizeof("MakGamGameLayer.dll"));

  BasePath = SDL_GetBasePath();
  BasePathLength = GetTerminatedStringLength(BasePath);

  char CopyFilePath[200];
  strncpy(
    CopyFilePath,
    BasePath, BasePathLength);
  strncpy(CopyFilePath + BasePathLength,
    "game.dll", sizeof("game.dll"));

  // Todo(sigmasleep): Find a better method?
  CopyFile(FilePath, CopyFilePath, FALSE);
  
  GameFunctions->Library = SDL_LoadObject(CopyFilePath);
  if(GameFunctions->Library)
  {
    GameFunctions->LoadGame = (load_game*)SDL_LoadFunction(
      GameFunctions->Library, "LoadGame"
    );
    GameFunctions->UpdateAndRenderGame = (update_and_render_game*)SDL_LoadFunction(
      GameFunctions->Library, "UpdateAndRenderGame"
    );
  }
  else
  {
    // Todo(sigmasleep): Add debug info/figure out wat do here
  }
}

internal void
UnloadGameFunctions(game_functions *GameFunctions)
{
  SDL_UnloadObject(GameFunctions->Library);
  GameFunctions->Library = NULL;
  GameFunctions->LoadGame = NULL;
  GameFunctions->UpdateAndRenderGame = NULL;
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
	SDL_GameController *Controller1 = SDL_GameControllerOpen(0);

  debug_tools DebugTools;
  DebugTools.DrawBox = DebugDrawBox;
  DebugTools.DrawCircle = DebugDrawCircle;
  DebugTools.DrawLine = DebugDrawLine;
  DebugTools.DrawSemiCircle = DebugDrawSemiCircle;
  DebugTools.DrawTriangle = DebugDrawTriangle;
  DebugTools.FillBox = DebugFillBox;
  DebugTools.SetColor = DebugSetColor;

  game_functions GameFunctions;
  LoadGameFunctions(&GameFunctions);

  float Dt;
  game_memory GameMemory;
  input_state Input = {};
  scene Scene;
  
	GameFunctions.LoadGame(&Scene, &DebugTools);

  GameMemory.Input = &Input;
  GameMemory.Scene = &Scene;

	uint32_t LastTime = SDL_GetTicks();	
	while(GlobalRunning) {
		Dt = (SDL_GetTicks() - LastTime) / 1000.f;
		LastTime = SDL_GetTicks();

    struct stat DLLInfo;

    char *BasePath = SDL_GetBasePath();
    int BasePathLength = GetTerminatedStringLength(BasePath);

    char FilePath[200];
    strncpy(
      FilePath,
      BasePath, BasePathLength);
    strncpy(FilePath + BasePathLength,
      "MakGamGameLayer.dll", sizeof("MakGamGameLayer.dll"));
    stat(FilePath, &DLLInfo);
    if(DLLInfo.st_mtime != GameFunctions.Timestamp)
    {
      UnloadGameFunctions(&GameFunctions);
      SDL_Delay(500);
      LoadGameFunctions(&GameFunctions);
      GameFunctions.LoadGame(GameMemory.Scene, &DebugTools);
      GameFunctions.Timestamp = DLLInfo.st_mtime;
    }

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

					// Note(sigmasleep): Deadzone value from mdsn for XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
					float Deadzone = 7849;
					// Note(sigmasleep): Normalization via division by int16 min/max values
					// Subtracting deadzone smooths the linear ramp that would otherwise be cut off.
					float NormalizedValue = ABS(Event.value) < Deadzone ? 0.f : Event.value < 0 ?
						(Event.value + Deadzone) / (32768.f - Deadzone) :
						(Event.value - Deadzone) / (32767.f - Deadzone);

					if(Event.which < CONTROLLER_MAX)
					{
						// Todo(sigmasleep): Add timing
						controller_state* Controller = &Input.Controllers[Event.which];

						switch(Event.axis)
						{
							case SDL_CONTROLLER_AXIS_LEFTX:
							{
								if(Controller->X != NormalizedValue)
								{
									Controller->XLastState = Controller->X;

									Controller->X = NormalizedValue;
									char output[255];
									snprintf(output, 255, "LeftX: %f\n",
											 NormalizedValue);
									OutputDebugStringA(output);
								}
							} break;
							case SDL_CONTROLLER_AXIS_LEFTY:
							{
								if(Controller->Y != NormalizedValue)
								{
									Controller->YLastState = Controller->Y;

									Controller->Y = NormalizedValue;
									char output[255];
									snprintf(output, 255, "LeftY: %f\n",
											 NormalizedValue);
									OutputDebugStringA(output);
								}
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
					if(Event.which < CONTROLLER_MAX)
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

		GameFunctions.UpdateAndRenderGame(&GameMemory, Dt);
		SDL_RenderPresent(GlobalRenderer);

		SDL_Delay(1);
	}

  UnloadGameFunctions(&GameFunctions);

	SDL_GameControllerClose(Controller1);

	SDL_Quit();

	return(0);
}
