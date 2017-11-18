#include "game.h"

#if 1
internal float
GetDistanceBetweenPoints(float X1, float Y1, float X2, float Y2)
{
  float X = X2 - X1;
  float Y = Y2 - Y1;
  return sqrtf(X * X + Y * Y);
}

//
//internal float
//GetAngleBetweenPoints(float X1, float Y1, float X2, float Y2)
//{
//	float Y = Y2 - Y1;
//	float X = X2 - X1;
//
//	return atan2f(Y, X) * RAD2DEG_CONSTANT;
//}

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
FillCollisionVectorCircleToLine(
  vector *CollisionVector,
  float CircleX, float CircleY, float CircleR,
  float X1, float Y1, float X2, float Y2
)
{
  vector Hypotenuse;
  Hypotenuse.X = CircleX - X1;
  Hypotenuse.Y = CircleY - Y1;

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
    Hypotenuse.X, Hypotenuse.Y, CircleR
  );
}

internal float
GetVectorMagnitude(vector *Input)
{
  return sqrtf(Input->X * Input->X + Input->Y * Input->Y);
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

  if(Direction1.X / Direction1.Y == Direction2.X / Direction2.Y)
  {
    return false;
  }

  *T2 = (Direction1.X * (Y3 - Y1) + Direction1.Y * (X1 - X3)) / (Direction2.X*Direction1.Y - Direction2.Y*Direction1.X);
  if(Direction1.X != 0)
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
    CollisionVector->X = -(X2 - X1) * T1;
    CollisionVector->Y = -(Y2 - Y1) * T1;
    return true;
  }

  return false;
}

internal bool
FillCollisionVectorCircleToLineWithVelocity(
  vector *CollisionVector,
  float CircleX, float CircleY,
  float CircleR,
  float CircleVX, float CircleVY,
  float LineX1, float LineY1,
  float LineX2, float LineY2
)
{
  vector CircleVelocity;
  CircleVelocity.X = CircleVX;
  CircleVelocity.Y = CircleVY;
  float CircleVelocityMagnitude =
    sqrtf(CircleVelocity.X * CircleVelocity.X +
          CircleVelocity.Y * CircleVelocity.Y);

  vector CircleDirection;
  CircleDirection.X = CircleVelocity.X / CircleVelocityMagnitude;
  CircleDirection.Y = CircleVelocity.Y / CircleVelocityMagnitude;

  vector CirclePathStart;
  CirclePathStart.X = CircleX - CircleR * CircleDirection.X;
  CirclePathStart.Y = CircleY - CircleR * CircleDirection.Y;
  //float CirclePathMagnitude = CircleVelocityMagnitude + CircleR * 2;
  vector CirclePathEnd;
  CirclePathEnd.X = CirclePathStart.X + CircleDirection.X * CircleR * 2;
  CirclePathEnd.Y = CirclePathStart.Y + CircleDirection.Y * CircleR * 2;

  if(FillCollisionVectorCircleToLine(
    CollisionVector,
    CircleX, CircleY,
    CircleR,
    LineX1, LineY1, LineX2, LineY2
  ))
  {
    return true;
  }

  if(FillCollisionVectorCircleToLine(
    CollisionVector,
    CircleX + CircleVX, CircleY + CircleVY,
    CircleR,
    LineX1, LineY1, LineX2, LineY2
  ))
  {
    return true;
  }

  return false;
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

  vector AnalogInput;
  AnalogInput.X = Controller->X;
  AnalogInput.Y = Controller->Y;
  {
    float MagnitudeSquared = AnalogInput.X * AnalogInput.X + AnalogInput.Y * AnalogInput.Y;
#define OUTERDEADZONE .98f    
    if(MagnitudeSquared > OUTERDEADZONE * OUTERDEADZONE)
    {
      // Note(sigmasleep): The analog coords are outside the outer deadzone
      float Magnitude = sqrtf(MagnitudeSquared);
      AnalogInput.X = AnalogInput.X / Magnitude;
      AnalogInput.Y = AnalogInput.Y / Magnitude;
    }
#define INNERDEADZONE .18f
    //8689.f / 32768.f
    else if(MagnitudeSquared < INNERDEADZONE * INNERDEADZONE)
    {
      // Note(sigmasleep): The analog coords are inside the the inner deadzone
      AnalogInput.X = 0;
      AnalogInput.Y = 0;
    }
    else
    {
      // Note(simasleep): The analog coords are between two deadzones and need to be rescaled.
      float Magnitude = sqrtf(MagnitudeSquared);
      float Scale = (Magnitude - INNERDEADZONE) / (OUTERDEADZONE - INNERDEADZONE);
      //Scale = CLIP(Scale, 0, 1);

      AnalogInput.X = AnalogInput.X / Magnitude * Scale;
      AnalogInput.Y = AnalogInput.Y / Magnitude * Scale;
    }
  }

  X += AnalogInput.X;
  Y += AnalogInput.Y;

  X = CLIP(X, -1.f, 1.f);
  Y = CLIP(Y, -1.f, 1.f);

  {
    float MagnitudeSquared = X * X + Y * Y;
    if(MagnitudeSquared > 1)
    {
      float Magnitude = sqrtf(MagnitudeSquared);
      X = X / Magnitude;
      Y = Y / Magnitude;
    }
  }

  Movement->X = X;
  Movement->Y = Y;
}

internal void
MovePlayer(hero *Hero, input_state *Input, float Dt)
{
  vector InputMovement;

  ProcessControllerMovement(&Input->Controllers[0], &InputMovement);

  Hero->Velocity.X = 100 * InputMovement.X * Dt;
  Hero->Velocity.Y = 100 * InputMovement.Y * Dt;

  if(Input->Controllers[0].WasMovedThisFrame ||
     Input->Controllers[0].Left.IsDown ||
     Input->Controllers[0].Right.IsDown ||
     Input->Controllers[0].Up.IsDown ||
     Input->Controllers[0].Down.IsDown)
  {
    if(InputMovement.X != 0 || InputMovement.Y != 0)
    {
      Hero->DirectionFacing = atan2f(Hero->Velocity.Y, Hero->Velocity.X) * RAD2DEG_CONSTANT;
    }
    else if(Input->Controllers[0].WasMovedThisFrame)
    {
      Hero->DirectionFacing = atan2f(Input->Controllers[0].Y, Input->Controllers[0].X) * RAD2DEG_CONSTANT;
    }
  }
}

internal void
ProcessDagger(dagger *Dagger, hero *Hero, float Dt)
{
  switch(Dagger->State)
  {
    case FIRED:
    {
      Dagger->Position.X += Dagger->Velocity.X * Dt;
      Dagger->Position.Y += Dagger->Velocity.Y * Dt;

      if(Dagger->Position.X < 0 || Dagger->Position.X > SCREEN_WIDTH
         || Dagger->Position.Y < 0 || Dagger->Position.Y > SCREEN_HEIGHT)
      {
        Dagger->State = STUCK;
        Dagger->BaddieStuckTo = NULL;
      }
    } break;
    case STUCK:
    {
      Dagger->Velocity.X /= 1.05f;
      Dagger->Velocity.Y /= 1.05f;
      if(Dagger->LastBattleChoice == PUSH && Dagger->Velocity.X*Dagger->Velocity.X + Dagger->Velocity.Y*Dagger->Velocity.Y < .3)
      {
        Dagger->State = RETURNING;
      }
      if(Dagger->BaddieStuckTo != NULL)
      {
        Dagger->BaddieStuckTo->Position.X += Dagger->Velocity.X * Dt;
        Dagger->BaddieStuckTo->Position.Y += Dagger->Velocity.Y * Dt;
        Dagger->Position.X = Dagger->BaddieStuckTo->Position.X;
        Dagger->Position.Y = Dagger->BaddieStuckTo->Position.Y;
      }
    } break;
    case RETURNING:
    {
      vector Direction;
      Direction.X = Hero->Position.X - Dagger->Position.X;
      Direction.Y = Hero->Position.Y - Dagger->Position.Y;
      NormalizeVector(&Direction, &Direction);

      Dagger->Velocity.X = Direction.X * 900;
      Dagger->Velocity.Y = Direction.Y * 900;

      if(GetDistanceBetweenPoints(
        Dagger->Position.X, Dagger->Position.Y,
        Hero->Position.X, Hero->Position.Y) < 720 * Dt)
      {
        Dagger->Position.X = Hero->Position.X;
        Dagger->Position.Y = Hero->Position.Y;
        Dagger->State = RESTING;
        Dagger->LastBattleChoice = NONE;
      }
      else
      {
        Dagger->Position.X += Dagger->Velocity.X * Dt;
        Dagger->Position.Y += Dagger->Velocity.Y * Dt;
      }
    } break;
    case RESTING:
    {
      Dagger->Position.X = Hero->Position.X;
      Dagger->Position.Y = Hero->Position.Y;
    } break;
  }
}

internal void
ProcessPlayerAction(hero *Hero, input_state *Input, float Dt)
{
  if(Input->Controllers[0].RightBumper.IsDown && Input->Controllers[0].RightBumper.WasReleasedSinceLastAction)
  {
    if(Hero->Dagger.State == RESTING)
    {
      Hero->Dagger.Velocity.X = 800 * cosf(Hero->DirectionFacing * DEG2RAD_CONSTANT);
      Hero->Dagger.Velocity.Y = 800 * sinf(Hero->DirectionFacing * DEG2RAD_CONSTANT);
      Hero->Dagger.State = FIRED;

    }
    Input->Controllers[0].RightBumper.WasReleasedSinceLastAction = false;
  }

  if(Input->Controllers[0].LeftBumper.IsDown && Input->Controllers[0].RightBumper.WasReleasedSinceLastAction)
  {
    if(Hero->Dagger.State == STUCK)
    {
      Hero->Dagger.State = RETURNING;
    }
    Input->Controllers[0].LeftBumper.WasReleasedSinceLastAction = false;
  }
}

internal void
PullDaggerBack(hero *Hero)
{
  Hero->Dagger.State = RETURNING;
}

internal void
WarpToBaddie(hero *Hero)
{
  Hero->Position.X = Hero->Dagger.BaddieStuckTo->Position.X;
  Hero->Position.Y = Hero->Dagger.BaddieStuckTo->Position.Y;
  Hero->Dagger.LastBattleChoice = WARPTO;
}

internal void
ProcessPlayerBattleAction(hero* Hero, input_state *Input, game_memory *Memory, float Dt)
{
  if(Input->Controllers[0].LeftBumper.IsDown && Input->Controllers[0].LeftBumper.WasReleasedSinceLastAction)
  {
    PullDaggerBack(Hero);
    Input->Controllers[0].LeftBumper.WasReleasedSinceLastAction = false;
    Memory->GameState = INGAME;
  }
  else if(Input->Controllers[0].RightBumper.IsDown && Input->Controllers[0].RightBumper.WasReleasedSinceLastAction)
  {
    switch(Memory->Scene->Hero.Dagger.LastBattleChoice)
    {
      case WARPTO:
      {
        Hero->Dagger.Velocity.X = 800 * cosf(Hero->DirectionFacing * DEG2RAD_CONSTANT);
        Hero->Dagger.Velocity.Y = 800 * sinf(Hero->DirectionFacing * DEG2RAD_CONSTANT);
        Hero->Dagger.LastBattleChoice = PUSH;
        Memory->GameState = INGAME;
      } break;
      case NONE:
      {
        Memory->BattleScreenTimer = 10;
        WarpToBaddie(Hero);
      } break;
    }
    Input->Controllers[0].RightBumper.WasReleasedSinceLastAction = false;
  }
}

internal void
RenderDebugArt(game_memory *Memory)
{
  if(Memory->GameState == BATTLESCREEN)
  {
    GlobalDebugTools->SetColor((int)(255 * Memory->BattleScreenTimer / 10.f), 0, 0, 255);
    GlobalDebugTools->FillBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  }
  else
  {
    GlobalDebugTools->SetColor(0, 200, 0, 255);
    GlobalDebugTools->FillBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    GlobalDebugTools->SetColor(0, 255, 0, 255);
    GlobalDebugTools->FillBox(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
                              SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  }

  vector Point[4];
  Point[0].X = 10;
  Point[0].Y = 10;
  Point[1].X = SCREEN_WIDTH - 10;
  Point[1].Y = 10;
  Point[2].X = SCREEN_WIDTH - 10;
  Point[2].Y = SCREEN_HEIGHT - 10;
  Point[3].X = 10;
  Point[3].Y = SCREEN_HEIGHT - 10;

  Memory->Scene->Hero.Position.X += Memory->Scene->Hero.Velocity.X;
  Memory->Scene->Hero.Position.Y += Memory->Scene->Hero.Velocity.Y;

  for(int PointIndex = 0; PointIndex < 4; PointIndex++)
  {
    vector CollisionVector;

    vector *Point1 = &Point[PointIndex];
    vector *Point2 = &Point[(PointIndex + 11) % 4];

    if(FillCollisionVectorCircleToLineWithVelocity(
      &CollisionVector,
      Memory->Scene->Hero.Position.X,
      Memory->Scene->Hero.Position.Y,
      Memory->Scene->Hero.Radius,
      Memory->Scene->Hero.Velocity.X,
      Memory->Scene->Hero.Velocity.Y,
      Point1->X, Point1->Y, Point2->X, Point2->Y
    ))
    {
      Memory->Scene->Hero.Position.X += CollisionVector.X;
      Memory->Scene->Hero.Position.Y += CollisionVector.Y;
    }
    GlobalDebugTools->DrawLine(Point1->X, Point1->Y, Point2->X, Point2->Y);
  }

  /*if (IsPointLeftHandToLine(
    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
    RandomPoint1.X, RandomPoint1.Y,
    RandomPoint2.X, RandomPoint2.Y))
  {
    GlobalDebugTools->SetColor(255, 0, 255, 255);
  }
  else
  {
    GlobalDebugTools->SetColor(0, 0, 255, 255);
  }*/

  GlobalDebugTools->SetColor(0, 0, 255, 255);
  GlobalDebugTools->DrawCircle(
    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
    Memory->Scene->Hero.Radius, 32);

  if(Memory->GameState == BATTLESCREEN)
  {
    RenderBaddie(Memory->Scene->Hero.Dagger.BaddieStuckTo);
  }
  else
  {
    for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
    {
      RenderBaddie(&Memory->Scene->Baddies[BaddieIndex]);
    }
  }

  GlobalDebugTools->DrawTriangle(
    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
    Memory->Scene->Hero.DirectionFacing,
    Memory->Scene->Hero.HalfHeight);

  vector RandomPoint3;
  RandomPoint3.X = Memory->Scene->Hero.Position.X;
  RandomPoint3.Y = Memory->Scene->Hero.Position.Y;

  vector HeroDirection;
  HeroDirection.X = cosf(Memory->Scene->Hero.DirectionFacing * DEG2RAD_CONSTANT);
  HeroDirection.Y = sinf(Memory->Scene->Hero.DirectionFacing * DEG2RAD_CONSTANT);

  vector RandomPoint4;
  RandomPoint4.X = RandomPoint3.X + HeroDirection.X * 60;
  RandomPoint4.Y = RandomPoint3.Y + HeroDirection.Y * 60;
  GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, RandomPoint4.X, RandomPoint4.Y);

  /*CollisionVector = {};

  if (FillCollisionVectorLineToLine(
    &CollisionVector,
    RandomPoint3.X, RandomPoint3.Y,
    RandomPoint4.X, RandomPoint4.Y,
    RandomPoint1.X, RandomPoint1.Y,
    RandomPoint2.X, RandomPoint2.Y))
  {

    vector Point;
    Point.X = RandomPoint3.X - CollisionVector.X;
    Point.Y = RandomPoint3.Y - CollisionVector.Y;

    GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, Point.X, Point.Y);

    GlobalDebugTools->SetColor(255, 255, 255, 255);

    GlobalDebugTools->DrawLine(
      RandomPoint4.X, RandomPoint4.Y,
      Point.X, Point.Y);
  }
  else
  {
    GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, RandomPoint4.X, RandomPoint4.Y);
  }*/

  GlobalDebugTools->SetColor(255, 255, 255, 255);
  GlobalDebugTools->DrawCircle(
    Memory->Scene->Hero.Dagger.Position.X,
    Memory->Scene->Hero.Dagger.Position.Y,
    Memory->Scene->Hero.Dagger.Radius,
    12
  );

  GlobalDebugTools->DrawCircle(
    80, 80, 50, 50
  );
  /*GlobalDebugTools->DrawCircle(
    80 + Memory->Input->Controllers[0].X * 50,
    80 + Memory->Input->Controllers[0].Y * 50,
    4, 10
  );*/
  vector ProcessedInput;
  ProcessControllerMovement(&Memory->Input->Controllers[0], &ProcessedInput);
  GlobalDebugTools->SetColor(255, 0, 255, 255);
  GlobalDebugTools->DrawCircle(
    80 + ProcessedInput.X * 50,
    80 + ProcessedInput.Y * 50,
    3, 10
  );
  GlobalDebugTools->SetColor(0, 0, 0, 255);
  GlobalDebugTools->DrawCircle(
    80 + Memory->Input->Controllers[0].X * 50,
    80 + Memory->Input->Controllers[0].Y * 50,
    3, 10
  );
  GlobalDebugTools->SetColor(255, 0, 0, 255);
  GlobalDebugTools->DrawCircle(
    80, 80, 2, 8
  );
  GlobalDebugTools->DrawCircle(
    80, 80, 1, 1
  );
}

/*
UpdateGame(game_memory Memory, float Dt)
{
  MovePlayer(&Memory->Scene->Hero, Memory->Input, Dt);
  ProcessDagger(&Memory->Scene->Hero.Dagger,
    &Memory->Scene->Hero, Dt);

  if (Memory->GameState == BATTLESCREEN)
  {
    CollideWithBaddie(&Memory->Scene->Hero, Memory->Scene->Hero.Dagger.BaddieStuckTo);
  }
  else
  {
    for (int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
    {
      CollideWithBaddie(&Memory->Scene->Hero, &Memory->Scene->Baddies[BaddieIndex]);
      CollideDaggerWithBaddie(&Memory->Scene->Hero.Dagger, &Memory->Scene->Baddies[BaddieIndex], Memory);
    }
  }
}
*/

internal void
ProcessLogics(game_memory *Memory, float Dt)
{
  switch(Memory->GameState)
  {
    case BATTLESCREEN:
    {
      Memory->TimeSpeed = 0.03f;
      Memory->BattleScreenTimer -= Dt;
      if(Memory->BattleScreenTimer <= 0)
      {
        Memory->GameState = INGAME;
        PullDaggerBack(&Memory->Scene->Hero);
      }
      ProcessPlayerBattleAction(&Memory->Scene->Hero, Memory->Input, Memory, Dt * Memory->TimeSpeed);
      BaddieMovement(Memory->Scene->Hero.Dagger.BaddieStuckTo, Dt * Memory->TimeSpeed);
    } break;
    case INGAME:
    {
      Memory->TimeSpeed = 1.f;
      ProcessPlayerAction(&Memory->Scene->Hero, Memory->Input, Dt);
      for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
      {
        BaddieMovement(&Memory->Scene->Baddies[BaddieIndex], Dt);
      }
    } break;
  }

  MovePlayer(&Memory->Scene->Hero, Memory->Input, Dt * Memory->TimeSpeed);
  ProcessDagger(&Memory->Scene->Hero.Dagger,
                &Memory->Scene->Hero, Dt * Memory->TimeSpeed);
}

internal void
ProcessPostCollisionLogics(game_memory *Memory, float Dt)
{

}

//internal void
//CollideWithBaddie(hero *Hero, baddie *Baddie)
//{
//
//}

internal
RESOLVE_COLLISION(ResolvePointerCollision)
{
  //game_memory *Memory = (game_memory *)_Memory;
  //hero *Hero = (hero *)This;
  baddie *Baddie = (baddie *)Other;

  switch(OtherType)
  {
    case BADDIE:
    {
      Baddie->Angle = atan2f(CollisionVector->Y, CollisionVector->X);
    } break;
    default:
    {
      // Todo(sigmasleep): Log this.
    } break;
  }
}

// Todo(sigmasleep): Although this will be removed,
// I should update the methods in here to use vector coords
internal void
CheckPointerCollision(hero* Hero, baddie* Baddie, game_memory *Memory)
{
  vector Direction;
  Direction.X = cosf(Hero->DirectionFacing * DEG2RAD_CONSTANT);
  Direction.Y = sinf(Hero->DirectionFacing * DEG2RAD_CONSTANT);

  vector PointerTipPosition;
  PointerTipPosition.X = Hero->Position.X + Direction.X * Hero->HalfHeight;
  PointerTipPosition.Y = Hero->Position.Y + Direction.Y * Hero->HalfHeight;

  float Distance = GetDistanceBetweenPoints(
    PointerTipPosition.X, PointerTipPosition.Y,
    Baddie->Position.X, Baddie->Position.Y
  );

  if(Distance < Baddie->Radius)
  {
    collision *Collision = &Memory->Collisions[Memory->CollisionsSize];

    Collision->Resolver = ResolvePointerCollision;
    Collision->This = (void *)Hero;
    Collision->Other = (void *)Baddie;
    Collision->OtherType = BADDIE;
    Collision->CollisionVector.X = Direction.X;
    Collision->CollisionVector.Y = Direction.Y;

    ++Memory->CollisionsSize;
  }
}

internal
RESOLVE_COLLISION(ResolveHeroCollision)
{
  //game_memory *Memory = (game_memory *)_Memory;
  //hero *Hero = (hero *)This;
  baddie *Baddie = (baddie *)Other;
  switch(OtherType)
  {
    case BADDIE:
    {
      Baddie->Position.X += CollisionVector->X;
      Baddie->Position.Y += CollisionVector->Y;
    } break;
    default:
    {
      // Todo(sigmasleep): Log this.
    } break;
  }
}

internal void
CheckHeroCollision(hero* Hero, baddie* Baddie, game_memory *Memory)
{
  vector CollisionVector;

  if(FillCollisionVectorCircleToCircle(&CollisionVector,
                                       Hero->Position.X, Hero->Position.Y, Hero->Radius,
                                       Baddie->Position.X, Baddie->Position.Y, Baddie->Radius))
  {
    collision *Collision = &Memory->Collisions[Memory->CollisionsSize];

    Collision->Resolver = ResolveHeroCollision;
    Collision->This = (void *)Hero;
    Collision->Other = (void *)Baddie;
    Collision->OtherType = BADDIE;
    Collision->CollisionVector.X = CollisionVector.X;
    Collision->CollisionVector.Y = CollisionVector.Y;

    ++Memory->CollisionsSize;
  }
}

internal
RESOLVE_COLLISION(ResolveDaggerCollision)
{
  game_memory *Memory = (game_memory *)_Memory;
  dagger *Dagger = (dagger *)This;
  switch(OtherType)
  {
    case BADDIE:
    {
      Dagger->State = STUCK;
      Dagger->BaddieStuckTo = (baddie *)Other;
      Memory->GameState = BATTLESCREEN;
      Memory->BattleScreenTimer = 10;
    } break;
    default:
    {
      // Todo(sigmasleep): Log this.
    } break;
  }
}

internal void
CheckDaggerCollision(dagger* Dagger, baddie* Baddie, game_memory *Memory)
{
  vector CollisionVector;
  if(Dagger->State == FIRED && FillCollisionVectorCircleToCircle(
    &CollisionVector,
    Dagger->Position.X, Dagger->Position.Y, Dagger->Radius,
    Baddie->Position.X, Baddie->Position.Y, Baddie->Radius
  ))
  {
    collision *Collision = &Memory->Collisions[Memory->CollisionsSize];

    Collision->Resolver = ResolveDaggerCollision;
    Collision->This = (void *)Dagger;
    Collision->Other = (void *)Baddie;
    Collision->OtherType = BADDIE;

    ++Memory->CollisionsSize;
  }
}

internal void
CheckCollisions(game_memory *Memory, float Dt)
{
  if(Memory->GameState == BATTLESCREEN)
  {
    CheckPointerCollision(&Memory->Scene->Hero, Memory->Scene->Hero.Dagger.BaddieStuckTo, Memory);
    CheckHeroCollision(&Memory->Scene->Hero, Memory->Scene->Hero.Dagger.BaddieStuckTo, Memory);
  }
  else
  {
    for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
    {
      CheckPointerCollision(&Memory->Scene->Hero, &Memory->Scene->Baddies[BaddieIndex], Memory);
      CheckHeroCollision(&Memory->Scene->Hero, &Memory->Scene->Baddies[BaddieIndex], Memory);
      CheckDaggerCollision(&Memory->Scene->Hero.Dagger, &Memory->Scene->Baddies[BaddieIndex], Memory);
    }
  }
}

internal void
ResolveCollisions(game_memory *Memory)
{
  for(int CollisionIndex = 0; CollisionIndex < Memory->CollisionsSize; ++CollisionIndex)
  {
    collision *Collision = &Memory->Collisions[CollisionIndex];
    resolve_collision *Resolve = Collision->Resolver;
    Resolve(Memory, Collision->This, Collision->Other, Collision->OtherType, &Collision->CollisionVector);
  }
}

// Todo(sigmasleep) Store these in entities?
//#define PROCESS_ENTITY_LOGIC(name) void name(game_memory *Memory, float Dt)
//#define CHECK_COLLISIONS(name) void name(game_memory *Memory) //create/throw away collision states each frame?
//#define PROCESS_ENTITY_POST_COLLISION_LOGIC(name) void name(game_memory *Memory, float Dt)
//ProcessLogic can add a collision check to collider stack?

#endif




//internal void
//RenderDebugArt(game_memory *Memory)
//{
//  if(Memory->GameState == BATTLESCREEN)
//  {
//    GlobalDebugTools->SetColor((int)(255 * Memory->BattleScreenTimer / 10.f), 0, 0, 255);
//    GlobalDebugTools->FillBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
//  }
//  else
//  {
//    GlobalDebugTools->SetColor(0, 200, 0, 255);
//    GlobalDebugTools->FillBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
//
//    GlobalDebugTools->SetColor(0, 255, 0, 255);
//    GlobalDebugTools->DrawBox(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4,
//                              SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
//  }
//  #if 1
//  vector Point[4];
//  Point[0].X = 10;
//  Point[0].Y = 10;
//  Point[1].X = SCREEN_WIDTH - 10;
//  Point[1].Y = 10;
//  Point[2].X = SCREEN_WIDTH - 10;
//  Point[2].Y = SCREEN_HEIGHT - 10;
//  Point[3].X = 10;
//  Point[3].Y = SCREEN_HEIGHT - 10;
//
//  Memory->Scene->Hero.Position.X += Memory->Scene->Hero.Velocity.X;
//  Memory->Scene->Hero.Position.Y += Memory->Scene->Hero.Velocity.Y;
//
//  for(int PointIndex = 0; PointIndex < 4; PointIndex++)
//  {
//    vector CollisionVector;
//
//    vector *Point1 = &Point[PointIndex];
//    vector *Point2 = &Point[(PointIndex + 11) % 4];
//
//    if(FillCollisionVectorCircleToLineWithVelocity(
//      &CollisionVector,
//      Memory->Scene->Hero.Position.X,
//      Memory->Scene->Hero.Position.Y,
//      Memory->Scene->Hero.Radius,
//      Memory->Scene->Hero.Velocity.X,
//      Memory->Scene->Hero.Velocity.Y,
//      Point1->X, Point1->Y, Point2->X, Point2->Y
//    ))
//    {
//      Memory->Scene->Hero.Position.X += CollisionVector.X;
//      Memory->Scene->Hero.Position.Y += CollisionVector.Y;
//    }
//    GlobalDebugTools->DrawLine(Point1->X, Point1->Y, Point2->X, Point2->Y);
//  }
//
//  /*if (IsPointLeftHandToLine(
//    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
//    RandomPoint1.X, RandomPoint1.Y,
//    RandomPoint2.X, RandomPoint2.Y))
//  {
//    GlobalDebugTools->SetColor(255, 0, 255, 255);
//  }
//  else
//  {
//    GlobalDebugTools->SetColor(0, 0, 255, 255);
//  }*/
//
//
//  if(Memory->GameState == BATTLESCREEN)
//  {
//    RenderBaddie(Memory->Scene->Hero.Dagger.BaddieStuckTo);
//  }
//  else
//  {
//    for(int BaddieIndex = 0; BaddieIndex < Memory->Scene->BaddieCount; BaddieIndex++)
//    {
//      RenderBaddie(&Memory->Scene->Baddies[BaddieIndex]);
//    }
//  }
//
//  GlobalDebugTools->DrawTriangle(
//    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
//    Memory->Scene->Hero.DirectionFacing,
//    Memory->Scene->Hero.HalfHeight);
//
//  vector RandomPoint3;
//  RandomPoint3.X = Memory->Scene->Hero.Position.X;
//  RandomPoint3.Y = Memory->Scene->Hero.Position.Y;
//
//  vector HeroDirection;
//  HeroDirection.X = cosf(Memory->Scene->Hero.DirectionFacing * DEG2RAD_CONSTANT);
//  HeroDirection.Y = sinf(Memory->Scene->Hero.DirectionFacing * DEG2RAD_CONSTANT);
//
//  vector RandomPoint4;
//  RandomPoint4.X = RandomPoint3.X + HeroDirection.X * 60;
//  RandomPoint4.Y = RandomPoint3.Y + HeroDirection.Y * 60;
//  GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, RandomPoint4.X, RandomPoint4.Y);
//
//  /*CollisionVector = {};
//
//  if (FillCollisionVectorLineToLine(
//    &CollisionVector,
//    RandomPoint3.X, RandomPoint3.Y,
//    RandomPoint4.X, RandomPoint4.Y,
//    RandomPoint1.X, RandomPoint1.Y,
//    RandomPoint2.X, RandomPoint2.Y))
//  {
//
//    vector Point;
//    Point.X = RandomPoint3.X - CollisionVector.X;
//    Point.Y = RandomPoint3.Y - CollisionVector.Y;
//
//    GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, Point.X, Point.Y);
//
//    GlobalDebugTools->SetColor(255, 255, 255, 255);
//
//    GlobalDebugTools->DrawLine(
//      RandomPoint4.X, RandomPoint4.Y,
//      Point.X, Point.Y);
//  }
//  else
//  {
//    GlobalDebugTools->DrawLine(RandomPoint3.X, RandomPoint3.Y, RandomPoint4.X, RandomPoint4.Y);
//  }*/
//
//  GlobalDebugTools->SetColor(255, 255, 255, 255);
//  GlobalDebugTools->DrawCircle(
//    Memory->Scene->Hero.Dagger.Position.X,
//    Memory->Scene->Hero.Dagger.Position.Y,
//    Memory->Scene->Hero.Dagger.Radius,
//    12
//  );
//
//  #endif
//  GlobalDebugTools->SetColor(0, 0, 255, 255);
//  GlobalDebugTools->DrawCircle(
//    Memory->Scene->Hero.Position.X, Memory->Scene->Hero.Position.Y,
//    Memory->Scene->Hero.Radius, 32);
//
//  GlobalDebugTools->SetColor(255, 255, 255, 255);
//  GlobalDebugTools->DrawCircle(
//    80, 80, 50, 50
//  );
//
//  vector ProcessedInput;
//  ProcessControllerMovement(&Memory->Input->Controllers[0], &ProcessedInput);
//  GlobalDebugTools->SetColor(255, 0, 255, 255);
//  GlobalDebugTools->DrawCircle(
//    80 + ProcessedInput.X * 50,
//    80 + ProcessedInput.Y * 50,
//    3, 10
//  );
//  GlobalDebugTools->SetColor(0, 0, 0, 255);
//  GlobalDebugTools->DrawCircle(
//    80 + Memory->Input->Controllers[0].X * 50,
//    80 + Memory->Input->Controllers[0].Y * 50,
//    3, 10
//  );
//  GlobalDebugTools->SetColor(255, 0, 0, 255);
//  GlobalDebugTools->DrawCircle(
//    80, 80, 2, 8
//  );
//  GlobalDebugTools->DrawCircle(
//    80, 80, 1, 1
//  );
//}
//Recieves game_memory *Memory && float Dt
extern "C"
UPDATE_GAME(UpdateGame)
{
  game_memory *GameMemory = (game_memory*)Memory->AllocatedSpace;
  GameMemory->CollisionsSize = 0;


  //GlobalDebugTools->SetColor(0, 255, 0, 0);
  //GlobalDebugTools->DrawLine(0, 300, 900, 300);
  //GlobalDebugTools->DrawBox(200, 200, 800, 300);
  //GlobalDebugTools->DrawSemiCircle(900, 900, 100, 18, 31, 140);
  //GlobalDebugTools->DrawCircle(800, 800, 80, 5);
  //GlobalDebugTools->DrawTriangle(500, 500, 45, 60);
  //GlobalDebugTools->FillBox(30, 30, 300, 90);


#if 1
  ProcessLogics(GameMemory, Dt);
  CheckCollisions(GameMemory, Dt);
  ResolveCollisions(GameMemory);
  ProcessPostCollisionLogics(GameMemory, Dt);
  //UpdateGame(Memory, Dt);

#endif
  RenderDebugArt(GameMemory);
}

extern "C"
LOAD_GAME(LoadGame)
{
  GlobalDebugTools = DebugTools;
  game_memory *GameMemory = (game_memory*)Memory->AllocatedSpace;

  scene *Scene = GameMemory->Scene;

  scene ClearedScene = {};
  *Scene = ClearedScene;

#if 1
  baddie Baddie = {};
  Baddie.Position.X = SCREEN_WIDTH / 4;
  Baddie.Position.Y = SCREEN_HEIGHT / 2;
  Baddie.Radius = 14;

  AddBaddieToScene(&Baddie, Scene);
  Baddie.Position.X += SCREEN_WIDTH / 4;
  AddBaddieToScene(&Baddie, Scene);
#endif

  Scene->Hero.Dagger.State = RESTING;
  Scene->Hero.Dagger.Radius = 16;
  Scene->Hero.Position.X = SCREEN_WIDTH / 2;
  Scene->Hero.Position.Y = SCREEN_HEIGHT / 2;
  Scene->Hero.DirectionFacing = 0;
  Scene->Hero.CurrentPathIndex = 0;
  Scene->Hero.Radius = 7;
  Scene->Hero.HalfHeight = acosf(30 * DEG2RAD_CONSTANT) * Scene->Hero.Radius * 2;
}

extern "C"
RELOAD_GAME(ReloadGame)
{
#if 1
  GlobalDebugTools = DebugTools;
  game_memory *GameMemory = (game_memory*)Memory->AllocatedSpace;

  scene *Scene = GameMemory->Scene;

  baddie Baddie = {};
  Baddie.Position.X = SCREEN_WIDTH / 4;
  Baddie.Position.Y = SCREEN_HEIGHT / 2;
  Baddie.Radius = 14;

  Scene->BaddieCount = 0;

  AddBaddieToScene(&Baddie, Scene);
  Baddie.Position.X += SCREEN_WIDTH / 4;
  AddBaddieToScene(&Baddie, Scene);

  Scene->Hero.Dagger.Radius = 16;
#endif
}
