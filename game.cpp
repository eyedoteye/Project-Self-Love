#include "game.h"
internal float
GetDistanceBetweenPoints(float X1, float Y1, float X2, float Y2)
{
	float X = X2 - X1;
	float Y = Y2 - Y1;
	return sqrtf(X * X + Y * Y);
}

internal float
GetAngleBetweenPoints(float X1, float Y1, float X2, float Y2)
{
	float Y = Y2 - Y1;
	float X = X2 - X1;

	return atan2f(Y, X) * RAD2DEG_CONSTANT;
}

internal void
AddBaddieToScene(baddie *Baddie, scene *Scene)
{
	Scene->Baddies[Scene->BaddieCount].Angle = Baddie->Angle;
	Scene->Baddies[Scene->BaddieCount].Position = Baddie->Position;
	Scene->Baddies[Scene->BaddieCount].Radius = Baddie->Radius;
	Scene->BaddieCount++;
}

internal void
BaddieMovement(baddie *Baddie, float Dt)
{
	float Distance = 10 * Dt;

	Baddie->Position.X += cosf(Baddie->Position.Y / 10) * Distance;
	Baddie->Position.Y += sinf(Baddie->Position.X / 10) * Distance;
}

internal void
RenderBaddie(baddie *Baddie)
{
	GlobalDebugTools->DrawSemiCircle(Baddie->Position.X, Baddie->Position.Y,
				   Baddie->Radius,
				   16, 32,
				   Baddie->Angle);
	GlobalDebugTools->DrawSemiCircle(Baddie->Position.X, Baddie->Position.Y,
				   Baddie->Radius,
				   16, 32,
				   Baddie->Angle + 180);
	GlobalDebugTools->DrawBox(Baddie->Position.X - Baddie->Radius, Baddie->Position.Y - Baddie->Radius,
			Baddie->Radius * 2, Baddie->Radius * 2);
}

internal bool
FillCollisionVectorCircleToCircle(
	vector *CollisionVector,
	float X1, float Y1, float R1,
	float X2, float Y2, float R2
)
{
	float XDistance = X2 - X1;
	float YDistance = Y2 - Y1;

	float RTotal = R1 + R2;
	float DistanceSquared = XDistance * XDistance + YDistance * YDistance;

	if(DistanceSquared < RTotal * RTotal)
	{
		float Distance = sqrtf(DistanceSquared);
		float UnitXDistance = XDistance / Distance;
		float UnitYDistance = YDistance / Distance;
		float CollisionDistance = RTotal - Distance;
		CollisionVector->X = UnitXDistance * CollisionDistance;
		CollisionVector->Y = UnitYDistance * CollisionDistance;

		return true;
	}

	return false;
}

internal bool
IsPointLeftHandToLine(
  float PointX, float PointY,
  float X1, float Y1,
  float X2, float Y2
)
{
  vector LocalPoint;
  LocalPoint.X = PointX - X1;
  LocalPoint.Y = PointY - Y1;

  vector LeftHandNormal;
  LeftHandNormal.X = Y2 - Y1;
  LeftHandNormal.Y = X1 - X2;

  float DotProduct = LeftHandNormal.X * LocalPoint.X + LeftHandNormal.Y * LocalPoint.Y;

  return DotProduct > 0;
}

internal bool
FillCollisionVectorLineToCircle(
	vector *CollisionVector,
	float X1, float Y1, float X2, float Y2,
	float X3, float Y3, float R3
)
{
	vector Hypotenuse;
	Hypotenuse.X = X3 - X1;
	Hypotenuse.Y = Y3 - Y1;

	vector Line;
	Line.X = X2 - X1;
	Line.Y = Y2 - Y1;

	float DotProduct = Hypotenuse.X * Line.X + Hypotenuse.Y * Line.Y;

	float LineLengthSquared = Line.X * Line.X + Line.Y * Line.Y;

	float ClippedDotProduct = CLIP(DotProduct, 0, LineLengthSquared);

	float ProjectionConstant = (ClippedDotProduct / LineLengthSquared);

	vector HypotenuseProjection;
	HypotenuseProjection.X = ProjectionConstant * Line.X;
	HypotenuseProjection.Y = ProjectionConstant * Line.Y;

	/*char output[255];
	snprintf(output, 255, "Line: %f %f\n",
	Line.X, Line.Y);
	OutputDebugStringA(output);
	snprintf(output, 255, "DotProduct: %f %f\n",
	DotProduct);
	OutputDebugStringA(output);
	snprintf(output, 255, "LineLengthSquared: %f\n",
	LineLengthSquared);
	OutputDebugStringA(output);
	snprintf(output, 255, "ProjectionConstant: %f\n",
	ProjectionConstant);
	OutputDebugStringA(output);
	snprintf(output, 255, "HypotenuseProjection: %f %f\n",
	HypotenuseProjection.X, HypotenuseProjection.Y);
	OutputDebugStringA(output);*/

	return FillCollisionVectorCircleToCircle(
		CollisionVector,
		HypotenuseProjection.X, HypotenuseProjection.Y, 0,
		Hypotenuse.X, Hypotenuse.Y, R3
	);
}

internal void
NormalizeVector(vector *Output, vector *Input)
{
  float Magnitude = sqrtf(Input->X * Input->X + Input->Y * Input->Y);
  Output->X = Input->X / Magnitude;
  Output->Y = Input->Y / Magnitude;
}

internal bool
FillCollisionTsLineToLine(
  float* T1, float* T2,
  float X1, float Y1,
  float X2, float Y2,
  float X3, float Y3,
  float X4, float Y4
)
{
  vector Direction1;
  Direction1.X = X2 - X1;
  Direction1.Y = Y2 - Y1;

  vector Direction2;
  Direction2.X = X4 - X3;
  Direction2.Y = Y4 - Y3;

  vector Direction1Normalized;
  vector Direction2Normalized;

  NormalizeVector(&Direction1Normalized, &Direction1);
  NormalizeVector(&Direction2Normalized, &Direction2);

  if (Direction1.X / Direction1.Y == Direction2.X / Direction2.Y)
  {
    return false;
  }

  *T2 = (Direction1.X * (Y3 - Y1) + Direction1.Y * (X1 - X3)) / (Direction2.X*Direction1.Y - Direction2.Y*Direction1.X);
  if (Direction1.X != 0)
  {
    *T1 = (X3 + Direction2.X*(*T2) - X1) / Direction1.X;
  }
  else
  {
    *T1 = (Y3 + Direction2.Y*(*T2) - Y1) / Direction1.Y;
  }

  return true;
}

internal bool
FillCollisionVectorLineToLine(
  vector *CollisionVector,
  float X1, float Y1,
  float X2, float Y2,
  float X3, float Y3,
  float X4, float Y4
)
{
  float T1;
  float T2;

  FillCollisionTsLineToLine(
    &T1, &T2,
    X1, Y1, X2, Y2,
    X3, Y3, X4, Y4
  );

  if(T1 > 0 && T1 < 1 &&
     T2 > 0 && T2 < 1)
  {
    CollisionVector->X = (X2 - X1) * T1;
    CollisionVector->Y = (Y2 - Y1) * T1;
    return true;
  }

  return false;
}

internal void
CollideWithBaddie(hero *Hero, baddie *Baddie)
{
	vector CollisionVector;

	if(FillCollisionVectorCircleToCircle(&CollisionVector,
	  Hero->Position.X, Hero->Position.Y, Hero->Radius,
		Baddie->Position.X, Baddie->Position.Y, Baddie->Radius))
	{
		Baddie->Position.X += CollisionVector.X;
		Baddie->Position.Y += CollisionVector.Y;
	}

	float X = Hero->Position.X + cosf(Hero->DirectionFacing * DEG2RAD_CONSTANT) * Hero->HalfHeight;
	float Y = Hero->Position.Y + sinf(Hero->DirectionFacing * DEG2RAD_CONSTANT) * Hero->HalfHeight;

	float Distance = GetDistanceBetweenPoints(
		X, Y,
		Baddie->Position.X, Baddie->Position.Y
	);

	if(Distance < Baddie->Radius)
	{
		float Direction = GetAngleBetweenPoints(
			X, Y,
			Baddie->Position.X, Baddie->Position.Y
		);

		Baddie->Angle = Direction;
	}
}

internal void
ProcessControllerMovement(controller_state *Controller, vector *Movement)
{
	float X = 0;
	float Y = 0;

	if(Controller->Left.IsDown)
	{
		X -= 1;
	}
	if(Controller->Right.IsDown)
	{
		X += 1;
	}
	if(Controller->Up.IsDown)
	{
		Y -= 1;
	}
	if(Controller->Down.IsDown)
	{
		Y += 1;
	}

	X += Controller->X;
	Y += Controller->Y;

	X = CLIP(X, -1.f, 1.f);
	Y = CLIP(Y, -1.f, 1.f);

	float MagnitudeSquared = X * X + Y * Y;
	if(MagnitudeSquared > 1)
	{
		float Magnitude = sqrtf(MagnitudeSquared);
		X = X / Magnitude;
		Y = Y / Magnitude;
	}

	Movement->X = X;
	Movement->Y = Y;
}

internal void
MovePlayer(hero *Hero, input_state *Input, float Dt)
{
	vector InputMovement;

	ProcessControllerMovement(&Input->Controllers[0], &InputMovement);

	Hero->Position.X += 100 * InputMovement.X * Dt;
	Hero->Position.Y += 100 * InputMovement.Y * Dt;

	if(InputMovement.Y != 0 || InputMovement.X != 0)
		Hero->DirectionFacing = atan2f(InputMovement.Y, InputMovement.X) * RAD2DEG_CONSTANT;
}

extern "C"
UPDATE_AND_RENDER_GAME(UpdateAndRenderGame)
{
	MovePlayer(&Memory->Scene->Hero, Memory->Input, Dt);
	for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
	{
		BaddieMovement(&Memory->Scene->Baddies[BaddieIndex], Dt);
	}

	// Note(sigmasleep): Seperate loops because movement updates should occur before collisions
	for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
	{
		CollideWithBaddie(&Memory->Scene->Hero, &Memory->Scene->Baddies[BaddieIndex]);
	}

	vector RandomPoint1;
	vector RandomPoint2;
	RandomPoint1.X = 100;
	RandomPoint1.Y = 200;
	RandomPoint2.X = 300;
	RandomPoint2.Y = 100;

	vector CollisionVector;

  if (FillCollisionVectorLineToCircle(
    &CollisionVector,
    RandomPoint1.X, RandomPoint1.Y, RandomPoint2.X, RandomPoint2.Y,
    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y, Memory->Scene->Hero.Radius
  ))
  {
    Memory->Scene->Hero.Position.X += CollisionVector.X;
    Memory->Scene->Hero.Position.Y += CollisionVector.Y;
  }

	GlobalDebugTools->SetColor(255, 0, 0, 255);
	GlobalDebugTools->FillBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
	GlobalDebugTools->SetColor(0, 255, 0, 255);
  GlobalDebugTools->FillBox(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
    SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  
	GlobalDebugTools->SetColor(0, 0, 255, 255);
	for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
	{
		RenderBaddie(&Memory->Scene->Baddies[BaddieIndex]);
	}

	GlobalDebugTools->DrawTriangle(
    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
		Memory->Scene->Hero.DirectionFacing,
	  Memory->Scene->Hero.HalfHeight);
	GlobalDebugTools->DrawCircle(
    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
    Memory->Scene->Hero.Radius, 32);
	GlobalDebugTools->DrawLine(RandomPoint1.X, RandomPoint1.Y, RandomPoint2.X, RandomPoint2.Y);

  vector RandomPoint3;
  RandomPoint3.X = Memory->Scene->Hero.Position.X;
  RandomPoint3.Y = Memory->Scene->Hero.Position.Y - 60;
  vector RandomPoint4;
  RandomPoint4.X = RandomPoint3.X;
  RandomPoint4.Y = RandomPoint3.Y + 120;

  GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, RandomPoint4.X, RandomPoint4.Y);
  CollisionVector = {};
  
  if(FillCollisionVectorLineToLine(
    &CollisionVector,
    RandomPoint3.X, RandomPoint3.Y,
    RandomPoint4.X, RandomPoint4.Y,
    RandomPoint1.X, RandomPoint1.Y,
    RandomPoint2.X, RandomPoint2.Y))
  {
    GlobalDebugTools->SetColor(255, 255, 255, 255);

    vector Point;
    Point.X = RandomPoint3.X + CollisionVector.X;
    Point.Y = RandomPoint3.Y + CollisionVector.Y;

    if(IsPointLeftHandToLine(
      Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
      RandomPoint1.X, RandomPoint1.Y,
      RandomPoint2.X, RandomPoint2.Y))
    {
      GlobalDebugTools->DrawLine(
        RandomPoint4.X, RandomPoint4.Y,
        Point.X, Point.Y);
    }
    else
    {
      GlobalDebugTools->DrawLine(
        RandomPoint3.X, RandomPoint3.Y,
        Point.X, Point.Y);
    }
  }
}

extern "C"
LOAD_GAME(LoadGame)
{
  GlobalDebugTools = DebugTools;

  scene *Scene = Memory->Scene;

  scene ClearedScene = {};
  *Scene = ClearedScene;

  baddie Baddie = {};
  Baddie.Position.X = SCREEN_WIDTH / 4;
  Baddie.Position.Y = SCREEN_HEIGHT / 2;
  Baddie.Radius = 14;

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

extern "C"
RELOAD_GAME(ReloadGame)
{
	GlobalDebugTools = DebugTools;

	scene *Scene = Memory->Scene;

  baddie Baddie = {};
  Baddie.Position.X = SCREEN_WIDTH / 4;
  Baddie.Position.Y = SCREEN_HEIGHT / 2;
  Baddie.Radius = 14;

  Scene->BaddieCount = 0;

  AddBaddieToScene(&Baddie, Scene);
  Baddie.Position.X += SCREEN_WIDTH / 4;
  AddBaddieToScene(&Baddie, Scene);
}