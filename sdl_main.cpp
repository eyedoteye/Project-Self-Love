// Todo(sigmasleep): just create my own string copy, this is stupid
#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <stdarg.h>

#define internal static
#define global_variable static
#define local_persist static

#include "game.h"

#include <windows.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define Kilobytes(Value) ((Value) * 1024L)
#define Megabytes(Value) (Kilobytes(Value) * 1024L)
#define Gigabytes(Value) (Megabytes(Value) * 1024L)

global_variable SDL_Renderer *GlobalRenderer;
global_variable bool GlobalRunning = true;
global_variable SDL_Window *GlobalWindow;

internal int
GetTerminatedStringLength(char* String)
{
	int Length = 0;
	while(*String != '\0')
	{
		Length++;
		String++;
	}

	return Length;
}

DEBUG_PRINT(DebugPrint)
{
	//int StringLength = GetTerminatedStringLength(OutputString);
	va_list Arguments;
	va_start(Arguments, OutputString);

	char OutputBuffer[255];
	vsnprintf(OutputBuffer, 255, OutputString, Arguments);
	
	OutputDebugStringA(OutputBuffer);
}

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
		Position2.X =
      X + cosf(RadAngle + PointNum / (float)TotalSegments * PI * 2) * Radius;
		Position2.Y =
      Y + sinf(RadAngle + PointNum / (float)TotalSegments * PI * 2) * Radius;

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
  reload_game *ReloadGame;
  update_and_render_game *UpdateAndRenderGame;
};

internal void
LoadGameFunctions(game_functions *GameFunctions)
{
  // Note(sigmasleep): Find a better way to do this?
  char* BasePath;
  BasePath = SDL_GetBasePath();
  int BasePathLength = GetTerminatedStringLength(BasePath);
  
  char FilePath[200];
  strncpy(
    FilePath,
    BasePath, BasePathLength);
  strncpy(FilePath + BasePathLength,
    "game.dll", sizeof("game.dll"));

  BasePath = SDL_GetBasePath();
  BasePathLength = GetTerminatedStringLength(BasePath);

  char CopyFilePath[200];
  strncpy(
    CopyFilePath,
    BasePath, BasePathLength);
  strncpy(CopyFilePath + BasePathLength,
    "gameruntime.dll", sizeof("gameruntime.dll"));

  // Note(sigmasleep): Does this ensure lock?
  struct stat ThrowAway;
  while(stat("lock.tmp", &ThrowAway) == 0)
  {
    SDL_Delay(1);
  }

  // Todo(sigmasleep): Find a x-platform method?
  CopyFile(FilePath, CopyFilePath, FALSE);
  
  GameFunctions->Library = SDL_LoadObject(CopyFilePath);
  if(GameFunctions->Library)
  {
    struct stat DLLInfo;
    stat(FilePath, &DLLInfo);
    GameFunctions->Timestamp = DLLInfo.st_mtime;

    GameFunctions->LoadGame = (load_game*)SDL_LoadFunction(
      GameFunctions->Library, "LoadGame"
    );
    GameFunctions->ReloadGame = (reload_game*)SDL_LoadFunction(
      GameFunctions->Library, "ReloadGame"
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

internal void
UpdateButton(button_state *Button, bool IsDown)
{
  Button->IsDown = IsDown;
  if (!IsDown)
  {
    Button->WasReleasedSinceLastAction = true;
  }
}

internal void
AllocateMemory(memory *Memory, int size)
{
  Memory->AllocatedSpace = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (Memory->AllocatedSpace)
  {
    Memory->Size = 0;
  }
  else
  {
    Memory->Size = -1;
  }
}

// Note(sigmasleep): This is copy pasted from SDL source in order to use the inner juices.s
struct _SDL_GameController
{
  SDL_Joystick *joystick; /* underlying joystick device */
 // int ref_count;
 // Uint8 hatState[4]; /* the current hat state for this controller */
 // struct _SDL_ControllerMapping mapping; /* the mapping object for this controller */
 // struct _SDL_GameController *next; /* pointer to next game controller we have allocated */
};

struct _SDL_Joystick
{
  SDL_JoystickID instance_id; /* Device instance, monotonically increasing from 0 */
};

int
main(int argc, char* argv[])
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
  SDL_GameController *Controllers[4] = {};
  
  int ControllerCount = SDL_NumJoysticks();
  ControllerCount = ControllerCount > CONTROLLER_MAX ? CONTROLLER_MAX : ControllerCount;

  for (int ControllerIndex = 0; ControllerIndex < CONTROLLER_MAX; ++ControllerIndex)
  {
    if (SDL_IsGameController(ControllerIndex))
    {
      Controllers[ControllerIndex] = SDL_GameControllerOpen(ControllerIndex);
    }
  }
  
  debug_tools DebugTools;
	DebugTools.Print = DebugPrint;
  DebugTools.DrawBox = DebugDrawBox;
  DebugTools.DrawCircle = DebugDrawCircle;
  DebugTools.DrawLine = DebugDrawLine;
  DebugTools.DrawSemiCircle = DebugDrawSemiCircle;
  DebugTools.DrawTriangle = DebugDrawTriangle;
  DebugTools.FillBox = DebugFillBox;
  DebugTools.SetColor = DebugSetColor;

  game_functions GameFunctions;
  LoadGameFunctions(&GameFunctions);

  memory Memory;
  AllocateMemory(&Memory, Megabytes(500));

  float Dt;
  input_state Input = {};
  
  for (int ControllerIndex = 0; ControllerIndex < CONTROLLER_MAX; ++ControllerIndex)
  {
    controller_state *Controller = &Input.Controllers[ControllerIndex];
    for (int ButtonIndex = 0; ButtonIndex < BUTTONCOUNT; ++ButtonIndex)
    {
      Controller->Buttons[ButtonIndex].WasReleasedSinceLastAction = true;
    }
  }

  scene Scene = {};
  game_memory *GameMemory = (game_memory*)Memory.AllocatedSpace;
  GameMemory->Input = &Input;
  GameMemory->Scene = &Scene;

	GameFunctions.LoadGame(&Memory, &DebugTools);

	uint32_t LastTime = SDL_GetTicks();	
	
  char *BasePath = SDL_GetBasePath();
  int BasePathLength = GetTerminatedStringLength(BasePath);

  char FilePath[200];
  strncpy(
    FilePath,
    BasePath, BasePathLength);
  strncpy(FilePath + BasePathLength,
    "game.dll", sizeof("game.dll"));
  while(GlobalRunning)
  {
		Dt = (SDL_GetTicks() - LastTime) / 1000.f;
		LastTime = SDL_GetTicks();

    struct stat DLLInfo;

    stat(FilePath, &DLLInfo);
    if(DLLInfo.st_mtime != GameFunctions.Timestamp)
    {
      UnloadGameFunctions(&GameFunctions);
      LoadGameFunctions(&GameFunctions);
      GameFunctions.ReloadGame(&Memory, &DebugTools);
    }

    {
      for (int ControllerIndex = 0; ControllerIndex < CONTROLLER_MAX; ++ControllerIndex)
      {
        Input.Controllers[ControllerIndex].WasMovedThisFrame = false;
      }
    }

		while(SDL_PollEvent(&e) != 0)
		{
			switch(e.type)
			{
				case SDL_QUIT:
				{
					GlobalRunning = false;
				} break;
        case SDL_CONTROLLERDEVICEADDED:
        {
          SDL_ControllerDeviceEvent Event = e.cdevice;
          DebugPrint("Added: %u", Event.which);
          if (ControllerCount < CONTROLLER_MAX)
          {
            //for (int ControllerIndex = 0; ControllerIndex < ControllerCount; ++ControllerIndex)
            //{
              if (Controllers[Event.which] == NULL)
              {
                Controllers[Event.which] = SDL_GameControllerOpen(Event.which);
                ++ControllerCount;
              }
              //else if(Controllers[ControllerIndex])
            //}
          }
          
        } break;
        case SDL_CONTROLLERDEVICEREMOVED:
        {
          SDL_ControllerDeviceEvent Event = e.cdevice;
          DebugPrint("Removed: %u", Event.which);
          for (int ControllerIndex = 0; ControllerIndex < ControllerCount; ++ControllerIndex)
          {
            if (Controllers[ControllerIndex]->joystick->instance_id == Event.which)
            {
              SDL_GameControllerClose(Controllers[ControllerIndex]);
              Controllers[ControllerIndex] = NULL;
              --ControllerCount;
            }
          }
        } break;
        case SDL_CONTROLLERAXISMOTION:
				{
					SDL_ControllerAxisEvent Event = e.caxis;

					// Note(sigmasleep): Deadzone value from mdsn for XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
          //#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
          //#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
          //#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30
					//float InnerDeadzone = 8689.f;
          // Note(sigmasleep): Normalization via division by int16 min/max values
					// Subtracting deadzone smooths the linear ramp that would otherwise be cut off.
          //float NormalizedValue = ABS(Event.value) < Deadzone ? 0.f : Event.value < 0 ?
					//	(Event.value + Deadzone) / (32768.f - Deadzone) :
					//	(Event.value - Deadzone) / (32767.f - Deadzone);

          int ActualControllerIndex = -1;
          for (int ControllerIndex = 0; ControllerIndex < ControllerCount; ++ControllerIndex)
          {
            if (Controllers[ControllerIndex] != NULL
                && Controllers[ControllerIndex]->joystick->instance_id == Event.which)
            {
              ActualControllerIndex = ControllerIndex;
              break;
            }
          }

          controller_state* Controller = &Input.Controllers[ActualControllerIndex];
					if(ActualControllerIndex != -1)
					{
						// Todo(sigmasleep): Add timing
            
            float NormalizedValue = Event.value < 0 ?
              Event.value / 32768.f :
              Event.value / 32767.f;

						switch(Event.axis)
						{
							case SDL_CONTROLLER_AXIS_LEFTX:
							{
                Controller->WasMovedThisFrame = Controller->WasMovedThisFrame || Controller->RawX != Event.value;
                Controller->RawX = Event.value;

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
                Controller->WasMovedThisFrame = Controller->WasMovedThisFrame || Controller->RawY != Event.value;
                Controller->RawY = Event.value;

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

          int ActualControllerIndex = -1;
          for (int ControllerIndex = 0; ControllerIndex < ControllerCount; ++ControllerIndex)
          {
            if (Controllers[ControllerIndex] != NULL
                && Controllers[ControllerIndex]->joystick->instance_id == Event.which)
            {
              ActualControllerIndex = ControllerIndex;
              break;
            }
          }

					if(ActualControllerIndex != -1)
					{
						// Todo(sigmasleep): Add timing
            controller_state* Controller = &Input.Controllers[ActualControllerIndex];

            switch(Event.button)
						{
							case SDL_CONTROLLER_BUTTON_DPAD_UP:
							{
								UpdateButton(&Controller->Up, IsDown);
							} break;
							case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							{
                UpdateButton(&Controller->Down, IsDown);
							} break;
							case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							{
                UpdateButton(&Controller->Left, IsDown);
							} break;
							case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							{
                UpdateButton(&Controller->Right, IsDown);
							} break;
              case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
              {
                UpdateButton(&Controller->LeftBumper, IsDown);
              } break;
              case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
              {
                UpdateButton(&Controller->RightBumper, IsDown);
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
              UpdateButton(&Controller->Up, IsDown);
						} break;
						case SDL_SCANCODE_DOWN:
						{
              UpdateButton(&Controller->Down, IsDown);
						} break;
						case SDL_SCANCODE_LEFT:
						{
              UpdateButton(&Controller->Left, IsDown); 
            } break;
						case SDL_SCANCODE_RIGHT:
						{
              UpdateButton(&Controller->Right, IsDown);
						} break;
					}
				} break;
			}
		}

		GameFunctions.UpdateAndRenderGame(&Memory, Dt);
		SDL_RenderPresent(GlobalRenderer);

		SDL_Delay(1);
  }

  UnloadGameFunctions(&GameFunctions);

	SDL_Quit();

	return(0);
}
