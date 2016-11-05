#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define internal static
#define global_variable static
#define local_persist static

#include "game.h"
#include "game.cpp"

#include <windows.h>
#include <stdio.h>
#include "SDL.h"

global_variable SDL_Renderer *GlobalRenderer;
global_variable bool GlobalRunning = true;
global_variable SDL_Window *GlobalWindow;

// Todo(sigmasleep): Remove from platform code once game has scene loading
void InitDebugScene(scene *Scene)
{
	baddie Baddie = {};

	Baddie.Position.X = SCREEN_WIDTH / 2;
	Baddie.Position.Y = SCREEN_HEIGHT / 2;
	Baddie.Radius = 14;

	scene ClearedScene = {};
	*Scene = ClearedScene;

	AddBaddieToScene(&Baddie, Scene);
	Baddie.Position.X += SCREEN_WIDTH / 4;
	AddBaddieToScene(&Baddie, Scene);

	Scene->Hero.Position.X = SCREEN_WIDTH / 4;
	Scene->Hero.Position.Y = SCREEN_HEIGHT / 4;
	Scene->Hero.DirectionFacing = 0;
	Scene->Hero.CurrentPathIndex = 0;
	Scene->Hero.Radius = 7;
	Scene->Hero.HalfHeight = acosf(30 * DEG2RAD_CONSTANT) * Scene->Hero.Radius * 2;
}

internal void
SetColor(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
	SDL_SetRenderDrawColor(GlobalRenderer, R, G, B, A);
}

internal void
DrawLine(float X1, float Y1, float X2, float Y2)
{
	SDL_RenderDrawLine(GlobalRenderer,
		(int)X1, (int)Y1,
		(int)X2, (int)Y2);
}

internal void
DrawSemiCircle(
	float X, float Y,
	float Radius,
	int Segments, int TotalSegments,
	float Angle)
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
		SDL_RenderDrawLine(GlobalRenderer,
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

internal void
DrawCircle(float X, float Y, float Radius, int Segments)
{
	DrawSemiCircle(X, Y, Radius, Segments, Segments, 0);
}

internal void
DrawTriangle(float X, float Y, float Angle, float HalfHeight)
{
	DrawSemiCircle(X, Y, HalfHeight, 3, 3, Angle);
}

internal void
DrawBox(float X, float Y, float Width, float Height)
{
	SDL_Rect DrawRect;
	DrawRect.x = (int)X;
	DrawRect.y = (int)Y;
	DrawRect.w = (int)Width;
	DrawRect.h = (int)Height;

	SDL_RenderDrawRect(GlobalRenderer, &DrawRect);
}

internal void
FillBox(float X, float Y, float Width, float Height)
{
	SDL_Rect FillRect;
	FillRect.x = (int)X;
	FillRect.y = (int)Y;
	FillRect.w = (int)Width;
	FillRect.h = (int)Height;

	SDL_RenderFillRect(GlobalRenderer, &FillRect);
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

	float Dt;
	input_state Input = {};

	// Todo(sigmasleep): Remove from platform code once game has scene loading
	scene Scene;
	InitDebugScene(&Scene);

	uint32_t LastTime = SDL_GetTicks();	
	while(GlobalRunning) {
		Dt = (SDL_GetTicks() - LastTime) / 1000.f;
		LastTime = SDL_GetTicks();

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
					// Note(sigmasleep): Normalization via division by int16 min/max values
					float Value = ABS(Event.value) < 7849 ? 0.f : 
						(Event.value < 0 ? Event.value / 32768.f : Event.value / 32767.f);

					if(Event.which < CONTROLLER_MAX)
					{
						// Todo(sigmasleep): Add timing
						controller_state* Controller = &Input.Controllers[Event.which];

						switch(Event.axis)
						{
							case SDL_CONTROLLER_AXIS_LEFTX:
							{
								if(Controller->X != Value)
								{
									Controller->XLastState = Controller->X;

									Controller->X = Value;
									char output[255];
									snprintf(output, 255, "LeftX: %f\n",
											 Value);
									OutputDebugStringA(output);
								}
							} break;
							case SDL_CONTROLLER_AXIS_LEFTY:
							{
								if(Controller->Y != Value)
								{
									Controller->YLastState = Controller->Y;

									Controller->Y = Value;
									char output[255];
									snprintf(output, 255, "LeftY: %f\n",
											 Value);
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

		RenderGame(&Input, &Scene, Dt);
		SDL_RenderPresent(GlobalRenderer);

		SDL_Delay(1);
	}

	SDL_GameControllerClose(Controller1);

	SDL_Quit();

	return(0);
}
