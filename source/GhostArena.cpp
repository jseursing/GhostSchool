#include "FruitController.h"
#include "GhostArena.h"
#include "GhostController.h"
#include "GhostSchool.h"
#include "OGLSpriteMgr.h"
#include "OGLSprite.h"
#include "OGLTimedSprite.h"
#include "PacmanController.h"
#include "SoundEngine.h"
#include <fstream>
#include <iostream>


// --------------------------------------------------------------------------------------
// Function: GhostArena
// Notes: None
// --------------------------------------------------------------------------------------
GhostArena* GhostArena::Instance(uint32_t programId)
{
  static GhostArena instance;
  if (nullptr == instance.Sprite)
  {
    instance.Sprite = 
      OGLSpriteMgr::Instance()->RegisterSprite(programId,
                                               "arenaTexture",
                                               GhostTypes::Sprite_Arena,
                                               1,
                                               GhostTypes::MAX_ARENA);

    instance.ArenaWidth = GhostSchool::Instance()->GetWidth();
    instance.ArenaHeight = instance.ArenaWidth * GhostTypes::DisplayRatio;
    instance.Sprite->SetSize(instance.ArenaWidth, instance.ArenaHeight);
    instance.Sprite->SetX(0);
    instance.Sprite->SetY(0);

    // Initialize pellet covers
    instance.PelletCovers = new OGLTimedSprite(programId, 
                                               GhostTypes::WindowWidth,
                                               GhostTypes::WindowHeight, 
                                               GhostTypes::ArenaWidth,
                                               GhostTypes::ArenaHeight,
                                               GhostTypes::HTileCount,
                                               GhostTypes::VTileCount);
    OGLSpriteMgr::Instance()->RegisterSprite(instance.PelletCovers, "pelletCover");
    instance.PelletCovers->SetSize(instance.ArenaWidth, instance.ArenaHeight);
    instance.PelletCovers->SetX(0);
    instance.PelletCovers->SetY(0);
    instance.PelletCovers->BackupImage();

    instance.SetLevel(0);
  }

  return &instance;
}

// --------------------------------------------------------------------------------------
// Function: LockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::LockUpdate()
{
  SkipUpdate = true;
}

// --------------------------------------------------------------------------------------
// Function: UnlockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::UnlockUpdate()
{
  SkipUpdate = false;
}

// --------------------------------------------------------------------------------------
// Function: SetLevel
// Notes: 
// The level data should be configured as an array of tiles. Each tile should cover
// 1-pixel as referenced by the original image. See GhostTypes.h for types.
// --------------------------------------------------------------------------------------
void GhostArena::SetLevel(uint32_t level)
{
  ArenaLock.lock();

  // Update member and arena
  RemainingPellets = 0;
  CurrentLevel = level;
  switch (CurrentLevel)
  {
  case 0:
    ArenaLevel = GhostTypes::ARENA0;
    Sprite->BackupImage();
    break;
  case 1:
    Sprite->RestoreImage();
    break;
  case 2:
    ArenaLevel = GhostTypes::ARENA1;
    Sprite->BackupImage();
    break;
  case 3:
  case 4:
    Sprite->RestoreImage();
    break;
  case 5:
    ArenaLevel = GhostTypes::ARENA2;
    Sprite->BackupImage();
    break;
  case 6:
  case 7:
  case 8:
    Sprite->RestoreImage();
    break;
  case 9:
    ArenaLevel = GhostTypes::ARENA3;
    Sprite->BackupImage();
    break;
  case 10:
  case 11:
  case 12:
    Sprite->RestoreImage();
    break;
  default:
    ArenaLevel = (GhostTypes::ARENA4 > (CurrentLevel % 8) ? GhostTypes::ARENA4 : 
                                                            GhostTypes::ARENA5);
    if (1 == (CurrentLevel % 4))
    {
      Sprite->BackupImage();
    }
    else
    {
      Sprite->RestoreImage();
    }
  }

  Sprite->SetActiveFrame(ArenaLevel);

  // Clear tile data
  TravelTiles.clear();

  // Load level information
  std::ifstream levelStream(GhostTypes::Data_Arena[ArenaLevel]);
  if (false == levelStream.is_open())
  {
    MessageBoxA(0, "Unable to locate level data", "Error", MB_OK);
    ArenaLock.unlock();
    return;
  }

  char buffer[1024 * 10] = { 0 }; // 10K of data

  // Populate Arena Tiles with the level data 
  bool syntaxError = false;
  uint32_t currentLine = 0;
  uint32_t currentPellet = 0;
  while (levelStream.getline(buffer, sizeof(buffer)))
  {
    if (('#' != buffer[0]) &&
        ('\n' != buffer[0]))
    {
      for (size_t i = 0; i < strlen(buffer); ++i)
      {
        switch (buffer[i])
        {
        case 'X':
          TravelTiles.push_back(GhostTypes::WALL);
          break;
        case 'p':
          ++RemainingPellets;
          TravelTiles.push_back(GhostTypes::SPELLET);
          break;
        case 'P':
          ++RemainingPellets;
          TravelTiles.push_back(GhostTypes::BPELLET);
          break;
        case 'T':
          TravelTiles.push_back(GhostTypes::PIPELINE);
          break;
        case '_':
          TravelTiles.push_back(GhostTypes::NOTHING);
          break;
        default:
          syntaxError = true;
        }
      }

      ++currentLine;
    }

    // Exit the loop if error
    if (true == syntaxError)
    {
      break;
    }
  }

  // Validate tiles
  if ((true == syntaxError) ||
      (GhostTypes::TotalTiles != TravelTiles.size()))
  {
    TravelTiles.clear();
    MessageBoxA(0, "Level Data corrupt", "Error", MB_OK);
    ArenaLock.unlock();
    return;
  }  

  // Calculate all legal moves
  MoveSet.clear();
  for (int32_t v = 0; v < static_cast<int32_t>(GhostTypes::VTileCount); ++v)
  {
    for (int32_t h = 0; h < static_cast<int32_t>(GhostTypes::HTileCount); ++h)
    {
      uint8_t bitmap = 0;
      
      // Check all possible directions, update bitmap if legal.
      if (GhostTypes::WALL != GetTileType(h, v))
      {
        if (static_cast<int32_t>(GhostTypes::HTileCount) > (h + 1)) // RIGHT
        {
          if (GhostTypes::WALL != GetTileType(h + 1, v))
          {
            bitmap |= (1 << GhostTypes::RIGHT);
          }
        }
        if (0 < h) // LEFT
        {
          if (GhostTypes::WALL != GetTileType(h - 1, v))
          {
            bitmap |= (1 << GhostTypes::LEFT);
          }
        }
        if (0 < v) // UP
        {
          if (GhostTypes::WALL != GetTileType(h, v - 1))
          {
            bitmap |= (1 << GhostTypes::UP);
          }
        }
        if (static_cast<int32_t>(GhostTypes::VTileCount) > (v + 1)) // DOWN
        {
          if (GhostTypes::WALL != GetTileType(h, v + 1))
          {
            bitmap |= (1 << GhostTypes::DOWN);
          }
        }
      }

      MoveSet.push_back(bitmap);
    }
  }

  ArenaLock.unlock();
}

// --------------------------------------------------------------------------------------
// Function: GetCurrentLevel
// Notes: None
// --------------------------------------------------------------------------------------
uint32_t GhostArena::GetCurrentLevel() const
{
  return CurrentLevel;
}

// --------------------------------------------------------------------------------------
// Function: GhostArena
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::Update(uint32_t tick)
{
  static uint32_t flashTick = 0;
  if (0 == (tick % GhostTypes::UpdateFreq))
  {
    if (false == SkipUpdate)
    {
      ++flashTick;
      // Every 5 ticks, enable the Pellet Covers
      if (0 == (flashTick % 20))
      {
        PelletCovers->StartRender(10);
      }

      // Retrieve player's current tile
      int32_t hTile = 0;
      int32_t vTile = 0;
      PacmanController::Instance()->GetTiles(hTile, vTile);

      // If the Tile type is a pellet, update the type and image buffer.
      // We are essentially drawing black squares over the original image.
      GhostTypes::TileTypeEnum type = GetTileType(hTile, vTile);
      if ((GhostTypes::SPELLET == type) ||
          (GhostTypes::BPELLET == type))
      {
        SetTileType(hTile, vTile, GhostTypes::NOTHING);

        // Draw a black square over pellet
        float hRatio = GhostTypes::ArenaWidth / GhostTypes::HTileCount;
        float vRatio = GhostTypes::ArenaHeight / GhostTypes::VTileCount;
        float arenaOffset = GhostTypes::ArenaHeight * ArenaLevel;
        uint32_t tileX = static_cast<uint32_t>(hTile * hRatio);
        uint32_t tileY = static_cast<uint32_t>(arenaOffset + (vTile * vRatio));
        Sprite->UpdateImage(tileX, 
                            tileY, 
                            static_cast<uint32_t>(hRatio), 
                            static_cast<uint32_t>(vRatio), 
                            1, 1, 1);

        // Add points and play sound effect
        switch (type)
        {
        case GhostTypes::SPELLET:
          GhostSchool::Instance()->IncreasePoints(10);
          SoundEngine::Instance()->Play(GhostTypes::AUDIO_SPELLET_ID, 0.9f);
          break;
        case GhostTypes::BPELLET:
          GhostSchool::Instance()->IncreasePoints(50);
          SoundEngine::Instance()->Play(GhostTypes::AUDIO_BPELLET_ID);

          // Update Ghost State
          GhostController::Instance()->SetGhostsVulnerable();
          break;
        }

        // Update pellets, update arena if done..
        --RemainingPellets; 
        if (0 == RemainingPellets) 
        {
          // Lock all updates
          PacmanController::Instance()->LockUpdate();
          GhostController::Instance()->LockUpdate();
          LockUpdate();

          // Reset player and ghosts
          GhostController::Instance()->ResetDestination(GhostTypes::BLINKY);
          GhostController::Instance()->ResetDestination(GhostTypes::PINKY);
          GhostController::Instance()->ResetDestination(GhostTypes::INKY);
          GhostController::Instance()->ResetDestination(GhostTypes::SUE);
          GhostController::Instance()->ResetStartPosition(GhostTypes::BLINKY);
          GhostController::Instance()->ResetStartPosition(GhostTypes::PINKY);
          GhostController::Instance()->ResetStartPosition(GhostTypes::INKY);
          GhostController::Instance()->ResetStartPosition(GhostTypes::SUE);
          PacmanController::Instance()->ResetDestination();
          PacmanController::Instance()->ResetStartPosition();

          // Set flashes..
          TransitionFrames = 100;
        }
        else 
        {
          if (false == FruitController::Instance()->IsVisible())
          {
            if (150 == RemainingPellets) // Enable fruit now
            {
              FruitController::Instance()->UnlockUpdate();
            }
          }
          else
          {
            if (100 == RemainingPellets) // Time to exit the arena
            {
              FruitController::Instance()->SetExitDirection();
            }
          }
        }
      }
    }
    else
    {
      if (0 != TransitionFrames)
      {
        --TransitionFrames;
        switch (TransitionFrames)
        {
          case 90:
          case 80:
          case 70:
          case 60:
          {
            Sprite->InvertImage();
          }
          break;
          case 50:
          {
            // Update arena
            SetLevel(++CurrentLevel);

            // Update level on HUD
            GhostSchool::Instance()->SetLevel(CurrentLevel);
          }
          break;
          case 0:
          {
            // Unlock
            PacmanController::Instance()->UnlockUpdate();
            GhostController::Instance()->UnlockUpdate();
            UnlockUpdate();
          }
          break;
        }
      }
    }
  }
}

// --------------------------------------------------------------------------------------
// Function: IsMovementLegal
// Notes: None
// --------------------------------------------------------------------------------------
bool GhostArena::IsMovementLegal(int32_t h, int32_t v, uint32_t direction)
{
  // Error check...
  if ((h >= static_cast<int32_t>(GhostTypes::HTileCount)) ||
      (v >= static_cast<int32_t>(GhostTypes::VTileCount)) ||
      (0 == MoveSet.size()))
  {
    return false;
  }

  ArenaLock.lock();

  // Move legality requires at least one space buffer around tiles
  bool legal = (1 << direction) & MoveSet[(GhostTypes::HTileCount * v) + h];

  ArenaLock.unlock();

  return legal;
}

// --------------------------------------------------------------------------------------
// Function: IsMovementLegal
// Notes: None
// --------------------------------------------------------------------------------------
bool GhostArena::IsTileLegal(int32_t h, int32_t v)
{
  // Error check...
  if ((h >= static_cast<int32_t>(GhostTypes::HTileCount)) ||
      (v >= static_cast<int32_t>(GhostTypes::VTileCount)) ||
      (0 == MoveSet.size()))
  {
    return false;
  }

  return (GhostTypes::WALL != GetTileType(h, v));
}

// --------------------------------------------------------------------------------------
// Function: PosToTile
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::PosToTile(float x, float y, int32_t& h, int32_t& v) const
{
  h = static_cast<int32_t>((x * GhostTypes::HTileCount) / ArenaWidth);
  v = static_cast<int32_t>((y * GhostTypes::VTileCount) / ArenaHeight);
}

// --------------------------------------------------------------------------------------
// Function: TileToPos
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::TileToPos(int32_t h, int32_t v, float& x, float& y) const
{
  x = (h * ArenaWidth) / GhostTypes::HTileCount;
  y = (v * ArenaHeight) / GhostTypes::VTileCount;
}  

// --------------------------------------------------------------------------------------
// Function: GetTileType
// Notes: None
// --------------------------------------------------------------------------------------
GhostTypes::TileTypeEnum GhostArena::GetTileType(int32_t h, int32_t v) const
{
  if (((v * GhostTypes::HTileCount) + h) >= TravelTiles.size())
  {
    return GhostTypes::PIPELINE;
  }

  return TravelTiles[(v * GhostTypes::HTileCount) + h];
}

// --------------------------------------------------------------------------------------
// Function: GetMoveSet
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::GetMoveSet(std::vector<uint8_t>& moveset)
{
  moveset = MoveSet;
}

// --------------------------------------------------------------------------------------
// Function: SetTileType
// Notes: None
// --------------------------------------------------------------------------------------
void GhostArena::SetTileType(int32_t h, int32_t v, GhostTypes::TileTypeEnum type)
{
  if (((v * GhostTypes::HTileCount) + h) < TravelTiles.size())
  {
    TravelTiles[(v * GhostTypes::HTileCount) + h] = type;
  }
}

// --------------------------------------------------------------------------------------
// Function: GhostArena
// Notes: None
// --------------------------------------------------------------------------------------
GhostArena::GhostArena() :
  Sprite(nullptr),
  ArenaWidth(0),
  ArenaHeight(0),
  TransitionFrames(0),
  ArenaLevel(GhostTypes::ARENA0),
  CurrentLevel(0),
  RemainingPellets(0),
  SkipUpdate(false)
{

}

// --------------------------------------------------------------------------------------
// Function: ~GhostArena
// Notes: None
// --------------------------------------------------------------------------------------
GhostArena::~GhostArena()
{

}