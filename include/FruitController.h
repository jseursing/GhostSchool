#pragma once
#include "GhostTypes.h"
#include <vector>

// Forward declarations
class OGLSprite;

class FruitController
{
public:

  enum DirectionType
  {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    NO_DIRECTION
  };

  static FruitController* Instance(uint32_t programId = -1);
  void LockUpdate();
  void UnlockUpdate();
  bool IsVisible() const;
  void SetExitDirection();
  void SetFruit(uint32_t id);
  void Update(uint32_t tick);
  void SetTiles(int32_t h, int32_t v);
  void GetTiles(int32_t& h, int32_t& v);
  void SetX(float x);
  float GetX() const;
  void SetY(float y);
  float GetY() const;

private:

  void SetRandomDirection();
  void FindPathAStar(int32_t targetH,
                     int32_t targetV, 
                     std::vector<DirectionType>& path);
  FruitController();
  ~FruitController();

  bool SkipUpdate;
  float X;
  float Y;
  float DestX;
  float DestY;
  float DeltaX;
  float DeltaY;
  int32_t HTile;
  int32_t VTile;
  uint32_t TransitionMax;
  uint32_t TransitionFrame;
  uint32_t FruitId;

  OGLSprite* Sprite;
  bool PlayAnimation;
  DirectionType Direction;
  DirectionType DirRequest;
  std::vector<DirectionType> DirSequence;
  GhostTypes::TileTypeEnum LastTile;
};