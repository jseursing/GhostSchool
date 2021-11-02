#pragma once
#include "GhostTypes.h"

// Forward declarations
class OGLSprite;

class PacmanController
{
public:

  static PacmanController* Instance(uint32_t programId = -1);
  void LockUpdate();
  void UnlockUpdate();
  void Update(uint32_t tick);
  void ProcessKey(GhostTypes::DirectionTypeEnum direction);
  void SetTiles(int32_t h, int32_t v);
  void GetTiles(int32_t& h, int32_t& v);
  void GetLastTiles(int32_t& h, int32_t& v);
  void SetX(float x);
  float GetX() const;
  void SetY(float y);
  float GetY() const;
  void SetDirection(GhostTypes::DirectionTypeEnum direction);
  GhostTypes::DirectionTypeEnum GetDirection() const;
  void InitiateDeathSequence();
  void ResetStartPosition();
  void ResetDestination();

private:

  PacmanController();
  ~PacmanController();

  bool SkipUpdate;

  float X;
  float Y;
  float DestX;
  float DestY;
  float DeltaX;
  float DeltaY;
  int32_t HTile;
  int32_t VTile;
  int32_t LastHTile;
  int32_t LastVTile;
  uint32_t TransitionMax;
  uint32_t TransitionFrame;

  OGLSprite* Sprite;
  bool PlayAnimation;
  uint32_t DeathTicks;
  GhostTypes::DirectionTypeEnum Direction;
  GhostTypes::DirectionTypeEnum DirRequest;
  GhostTypes::TileTypeEnum LastTile;

  uint32_t AnimationTick;
  static const uint32_t AnimationCount = 3;
};