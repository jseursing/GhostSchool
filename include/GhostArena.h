#pragma once
#include "GhostTypes.h"
#include <mutex>
#include <stdint.h>
#include <vector>

// Forward declarations
class OGLSprite;
class OGLTimedSprite;


// This class will be used to represent the arena
// which the player and ghosts will travel/draw on.
class GhostArena
{
public:
  
  static GhostArena* Instance(uint32_t programId = -1);
  void LockUpdate();
  void UnlockUpdate();
  void SetLevel(uint32_t level);
  uint32_t GetCurrentLevel() const;
  void Update(uint32_t tick);
  bool IsMovementLegal(int32_t h, int32_t v, uint32_t direction);
  bool IsTileLegal(int32_t h, int32_t v);
  void PosToTile(float x, float y, int32_t& h, int32_t& v) const;
  void TileToPos(int32_t h, int32_t v, float& x, float& y) const;
  GhostTypes::TileTypeEnum GetTileType(int32_t h, int32_t v) const;
  void GetMoveSet(std::vector<uint8_t>& moveset);

private:
  
  void SetTileType(int32_t h, int32_t v, GhostTypes::TileTypeEnum type);
  GhostArena();
  ~GhostArena();


  bool SkipUpdate;
  OGLSprite* Sprite;
  OGLTimedSprite* PelletCovers;
  GhostTypes::ArenaEnum ArenaLevel;
  uint32_t CurrentLevel;
  float ArenaWidth;
  float ArenaHeight;
  int32_t RemainingPellets;
  uint32_t TransitionFrames;

  std::vector<GhostTypes::TileTypeEnum> TravelTiles;
  std::vector<uint8_t> MoveSet;
  std::mutex ArenaLock;  

#if _DEBUG
  friend class GhostSchool;
#endif
};