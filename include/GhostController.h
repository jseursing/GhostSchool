#pragma once
#include "GhostTypes.h"
#include <set>
#include <stdint.h>

// Forward declarations
class OGLSprite;

class GhostController
{
public:

  enum StateEnum
  {
    ENCASED,
    ENTERING,
    ROAMING,
    CHASING,
    RETREATING,
    EXITING
  };

  static GhostController* Instance(uint32_t programId = -1);
  void LockUpdate();
  void UnlockUpdate();
  void Update(uint32_t tick);
  void SetTiles(GhostTypes::GhostTypeEnum ghost, int32_t h, int32_t v);
  void GetTiles(GhostTypes::GhostTypeEnum ghost, int32_t& h, int32_t& v);
  void SetX(GhostTypes::GhostTypeEnum ghost, float x);
  float GetX(GhostTypes::GhostTypeEnum ghost) const;
  void SetY(GhostTypes::GhostTypeEnum ghost, float y);
  float GetY(GhostTypes::GhostTypeEnum ghost) const;
  void ResetStartPosition(GhostTypes::GhostTypeEnum ghost);
  void ResetDestination(GhostTypes::GhostTypeEnum ghost);
  void SetGhostsVulnerable();

private:

  bool IsMovementLegal(GhostTypes::GhostTypeEnum ghost, 
                       GhostTypes::DirectionTypeEnum direction,
                       int32_t offset = 0);
  bool IsMovementLegal(GhostTypes::GhostTypeEnum ghost,
                       GhostTypes::DirectionTypeEnum direction,
                       int32_t hTile,
                       int32_t vTile);
  void UpdateDirection(GhostTypes::GhostTypeEnum ghost);
  void SetRandDirection(GhostTypes::GhostTypeEnum ghost);
  void SetExitDirection(GhostTypes::GhostTypeEnum ghost);
  void SetEnterDirection(GhostTypes::GhostTypeEnum ghost);
  void SetTargetDirection(GhostTypes::GhostTypeEnum ghost, 
                          int32_t targetH, 
                          int32_t targetV, 
                          bool towards);
  void FindPathBFS(GhostTypes::GhostTypeEnum ghost,
                   int32_t targetH,
                   int32_t targetV, 
                   std::vector<GhostTypes::DirectionTypeEnum>& path);
  void FindPathDFS(GhostTypes::GhostTypeEnum ghost,
                   int32_t targetH,
                   int32_t targetV, 
                   std::vector<GhostTypes::DirectionTypeEnum>& path);
  void FindPathAStar(GhostTypes::GhostTypeEnum ghost,
                     int32_t targetH,
                     int32_t targetV, 
                     std::vector<GhostTypes::DirectionTypeEnum>& path);
  bool FindPathBFS(GhostTypes::GhostTypeEnum ghost,
                   int32_t targetH, 
                   int32_t targetV, 
                   std::vector<GhostTypes::DirectionTypeEnum>& path,
                   std::vector<uint16_t>& usedTiles);
  bool FindPathDFS(GhostTypes::GhostTypeEnum ghost,
                   int32_t targetH, 
                   int32_t targetV, 
                   std::vector<GhostTypes::DirectionTypeEnum>& path,
                   std::vector<uint16_t>& usedTiles);
  void SetState(GhostTypes::GhostTypeEnum ghost, StateEnum state);
  void UpdateState(GhostTypes::GhostTypeEnum ghost);

  GhostController();
  ~GhostController();

  bool SkipUpdate;

  float X[GhostTypes::MAX_GHOST];
  float Y[GhostTypes::MAX_GHOST];
  float DestX[GhostTypes::MAX_GHOST];
  float DestY[GhostTypes::MAX_GHOST];
  float DeltaX[GhostTypes::MAX_GHOST];
  float DeltaY[GhostTypes::MAX_GHOST];
  int32_t HTile[GhostTypes::MAX_GHOST];
  int32_t VTile[GhostTypes::MAX_GHOST];
  uint32_t TransitionMax[GhostTypes::MAX_GHOST];
  uint32_t TransitionFrame[GhostTypes::MAX_GHOST];

  OGLSprite* Sprite[GhostTypes::MAX_GHOST];
  bool PlayAnimation[GhostTypes::MAX_GHOST];
  GhostTypes::DirectionTypeEnum Direction[GhostTypes::MAX_GHOST];
  GhostTypes::DirectionTypeEnum DirRequest[GhostTypes::MAX_GHOST];
  uint32_t MaximumSequence[GhostTypes::MAX_GHOST];
  std::vector<GhostTypes::DirectionTypeEnum> DirSequence[GhostTypes::MAX_GHOST];

  uint32_t ScoreMultiplier;
  bool SetVulnerable[GhostTypes::MAX_GHOST];
  StateEnum CurrentState[GhostTypes::MAX_GHOST];
  StateEnum LastState[GhostTypes::MAX_GHOST];
  uint32_t RemainingState[GhostTypes::MAX_GHOST];
  GhostTypes::TileTypeEnum LastTile[GhostTypes::MAX_GHOST];
  uint32_t AnimationOffset[GhostTypes::NO_DIRECTION];
  uint32_t AnimationTick[GhostTypes::MAX_GHOST];
  static const uint32_t AnimationCount[GhostTypes::NO_DIRECTION];
};