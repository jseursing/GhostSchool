#include "GhostArena.h"
#include "GhostController.h"
#include "GhostSchool.h"
#include "OGLSpriteMgr.h"
#include "OGLSprite.h"
#include "PacmanController.h"
#include "SoundEngine.h"
#include <ctime>

// Static definitions
const uint32_t GhostController::AnimationCount[GhostTypes::NO_DIRECTION] = 
  { 2, 2, 2, 2, 4, 1 };

// --------------------------------------------------------------------------------------
// Function: Instance
// Notes: None
// --------------------------------------------------------------------------------------
GhostController* GhostController::Instance(uint32_t programId)
{
  static GhostController instance;
  if ((nullptr == instance.Sprite[GhostTypes::BLINKY]) &&
      (nullptr == instance.Sprite[GhostTypes::PINKY]) &&
      (nullptr == instance.Sprite[GhostTypes::INKY]) &&
      (nullptr == instance.Sprite[GhostTypes::SUE]))
  {
    srand(static_cast<uint32_t>(time(0)));

    for (size_t i = GhostTypes::BLINKY; i <= GhostTypes::SUE; ++i)
    {
      instance.Sprite[i] =
        OGLSpriteMgr::Instance()->RegisterSprite(programId,
                                                 "ghostTexture",
                                                 GhostTypes::Sprite_Ghost[i],
                                                 16,
                                                 1);
      instance.Sprite[i]->SetSize(GhostTypes::PlayerSize, 
                                  GhostTypes::PlayerSize);
      instance.ResetStartPosition(static_cast<GhostTypes::GhostTypeEnum>(i));
    }
  }

  return &instance;
}

// --------------------------------------------------------------------------------------
// Function: LockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::LockUpdate()
{
  SkipUpdate = true;
}

// --------------------------------------------------------------------------------------
// Function: UnlockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::UnlockUpdate()
{
  SkipUpdate = false;
}

// --------------------------------------------------------------------------------------
// Function: Update
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::Update(uint32_t tick)
{
  if (false == SkipUpdate)
  {
    for (size_t i = GhostTypes::BLINKY; i <= GhostTypes::SUE; ++i)
    {
      GhostTypes::GhostTypeEnum ghost = static_cast<GhostTypes::GhostTypeEnum>(i);

      int32_t hTile = 0;
      int32_t vTile = 0;
      GetTiles(ghost, hTile, vTile);

      // If the current state is ROAMING or CHASING, check to see
      // if the current ghost caught up to the player. Initiate
      // death sequence if this is the case.
      if ((EXITING != CurrentState[i]) &&
          (ENCASED != CurrentState[i]))
      {
        int32_t pTileH = 0;
        int32_t pTileV = 0;
        PacmanController::Instance()->GetTiles(pTileH, pTileV);

        // There's a "rare" case where player and ghost detection isn't
        // accurate due to opposing directions and frame timing. Hopefully
        // this fix proves worthwhile.
        bool collision = (hTile == pTileH) &&
                         (vTile == pTileV);
        if (false == collision)
        {
          // Check to see if we are one tile away in distance and opposing
          // directions in order to set this flag.
          GhostTypes::DirectionTypeEnum pDirection = 
            PacmanController::Instance()->GetDirection();
          if (((GhostTypes::RIGHT == Direction[ghost]) &&
               (GhostTypes::LEFT == pDirection)) ||
              ((GhostTypes::LEFT == Direction[ghost]) &&
               (GhostTypes::RIGHT == pDirection)) ||
              ((GhostTypes::UP == Direction[ghost]) &&
               (GhostTypes::DOWN == pDirection)) ||
              ((GhostTypes::DOWN == Direction[ghost]) &&
               (GhostTypes::UP == pDirection)))
          {
            PacmanController::Instance()->GetLastTiles(pTileH, pTileV);
            collision = (hTile == pTileH) &&
                        (vTile == pTileV);
          }
        }

        if (true == collision)
        {
          // IF the current state is not Vulnerable, initiate the death sequence.
          if (GhostTypes::VULNERABLE != CurrentState[ghost])
          {
            // Lock updates. Let Player controller perform audio
            // and animation, reset ghosts/player, and enable updates.
            PacmanController::Instance()->InitiateDeathSequence();
            ResetStartPosition(GhostTypes::BLINKY);
            ResetDestination(GhostTypes::BLINKY);
            ResetStartPosition(GhostTypes::PINKY);
            ResetDestination(GhostTypes::PINKY);
            ResetStartPosition(GhostTypes::INKY);
            ResetDestination(GhostTypes::INKY);
            ResetStartPosition(GhostTypes::SUE);
            ResetDestination(GhostTypes::SUE);
            LockUpdate();
            return;
          }
          else
          {
            // The ghost is vulnerable and is now eaten. Play the eaten
            // sound and update state.
            SoundEngine::Instance()->Play(GhostTypes::AUDIO_EAT_ID);
            GhostSchool::Instance()->SetDelayedLockout(50);
            SetState(ghost, EXITING);

            // Increase score and adjust multiplier
            GhostSchool::Instance()->IncreasePoints
              (static_cast<uint32_t>(100 * pow(2, ScoreMultiplier)));
            GhostSchool::Instance()->DisplayScoreSprite(GetX(ghost), 
                                                        GetY(ghost), 
                                                        false, 
                                                        ScoreMultiplier - 1);
            ++ScoreMultiplier;

            // Update animation
            uint32_t activeFrame = AnimationOffset[GhostTypes::EATEN] +
                                   (Direction[ghost] % AnimationCount[GhostTypes::EATEN]);
            Sprite[ghost]->SetActiveFrame(activeFrame);
          }

          // Reset movement
          TransitionFrame[ghost] = 0;
        }
      }

      // Animation
      if (0 == (tick % GhostTypes::AnimationFreq))
      {
        if (true == PlayAnimation[ghost])
        {
          uint32_t activeFrame = 0;
          switch (CurrentState[ghost])
          {
          case ENCASED:
          case ROAMING:
          case CHASING:
          case ENTERING:
            activeFrame = AnimationOffset[Direction[ghost]] +
              (AnimationTick[ghost] % AnimationCount[ghost]);
            break;
          case RETREATING:
            activeFrame = AnimationOffset[GhostTypes::VULNERABLE] +
              (AnimationTick[ghost] % AnimationCount[GhostTypes::VULNERABLE]);
            break;
          case EXITING:
            activeFrame = AnimationOffset[GhostTypes::EATEN] +
              (Direction[ghost] % AnimationCount[GhostTypes::EATEN]);
            break;
          }

          Sprite[ghost]->SetActiveFrame(activeFrame);
          ++AnimationTick[ghost];
        }
      }

      // This is where we update decision and location
      {
        // Handle state changes based on time. Additionally update
        // the Direction Sequence.
        if (0 == TransitionFrame[ghost])
        {
          UpdateState(ghost);
        }

        // Only update direction if legal and pixels line up perfectly.
        if ((GhostTypes::NO_DIRECTION != DirRequest[ghost]) &&
            (0 == TransitionFrame[ghost]))
        {
          if (true == IsMovementLegal(ghost, DirRequest[i]))
          {
            Direction[ghost] = DirRequest[ghost];
            DirRequest[ghost] = GhostTypes::NO_DIRECTION;
          }
        }

        if (GhostTypes::NO_DIRECTION != Direction[ghost])
        {
          int32_t hTileD = 0;
          int32_t vTileD = 0;

          PlayAnimation[ghost] = (0 != TransitionFrame[ghost]) ||
                                 (IsMovementLegal(ghost, Direction[ghost]));
          if (true == PlayAnimation[ghost])
          {
            switch (Direction[ghost])
            {
            case GhostTypes::LEFT:
              hTileD = hTile - GhostTypes::TileDelta;
              vTileD = vTile;
              break;
            case GhostTypes::UP:
              vTileD = vTile - GhostTypes::TileDelta;
              hTileD = hTile;
              break;
            case GhostTypes::RIGHT:
              hTileD = hTile + GhostTypes::TileDelta;
              vTileD = vTile;
              break;
            case GhostTypes::DOWN:
              vTileD = vTile + GhostTypes::TileDelta;
              hTileD = hTile;
              break;
            }

            if ((GhostTypes::PIPELINE == 
                 GhostArena::Instance()->GetTileType(hTileD, vTileD)) &&
                (GhostTypes::PIPELINE != LastTile[i]))
            {
              // Immediately update position
              TransitionFrame[ghost] = 0;
              LastTile[ghost] = GhostTypes::PIPELINE;
              hTileD = (0 == hTileD ? GhostTypes::HTileCount - 1 : 0);

              float currX = 0;
              float currY = 0;
              GhostArena::Instance()->TileToPos(hTileD, vTileD, currX, currY);

              SetX(ghost, currX);
              SetY(ghost, currY);
              SetTiles(ghost, hTileD, vTileD);

              // Force the ghost to move in the same direction for two more steps.
              DirSequence[ghost].clear();
              DirSequence[ghost].push_back(Direction[ghost]);
              DirSequence[ghost].push_back(Direction[ghost]);

              return;
            }

            if (0 == TransitionFrame[ghost])
            {
              ++TransitionFrame[ghost];

              // Update position
              GhostArena::Instance()->TileToPos(hTileD, vTileD, DestX[ghost], DestY[ghost]);
              LastTile[ghost] = GhostArena::Instance()->GetTileType(hTileD, vTileD);
              DeltaX[ghost] = (DestX[ghost] - GetX(ghost)) / TransitionMax[ghost];
              DeltaY[ghost] = (DestY[ghost] - GetY(ghost)) / TransitionMax[ghost];

              SetX(ghost, GetX(ghost));
              SetY(ghost, GetY(ghost));
              SetTiles(ghost, hTileD, vTileD);

              // Play audio depending on state
              switch (CurrentState[ghost])
              {
              case ENCASED:
              case ENTERING:
              case ROAMING:
              case CHASING:
                SoundEngine::Instance()->Play(GhostTypes::AUDIO_GHOST_ID[ghost], 0.9f);
                break;
              case RETREATING:
                if (false == SoundEngine::Instance()->IsActive(GhostTypes::AUDIO_VULN_ID))
                {
                  SoundEngine::Instance()->Play(GhostTypes::AUDIO_VULN_ID);
                }
                break;
              case EXITING:
                SoundEngine::Instance()->Play(GhostTypes::AUDIO_RETREAT_ID, 0.9f);
                break;
              }
            }
            else if (TransitionMax[ghost] == TransitionFrame[ghost])
            {
              TransitionFrame[ghost] = 0;
              SetX(ghost, DestX[ghost]);
              SetY(ghost, DestY[ghost]);
            }
            else
            {
              ++TransitionFrame[ghost];
              SetX(ghost, (GetX(ghost) + DeltaX[ghost]));
              SetY(ghost, (GetY(ghost) + DeltaY[ghost]));
            }
          }
        }
      }
    }
  }
}

// --------------------------------------------------------------------------------------
// Function: SetTiles
// Notes: Should only be called to set exact position.
// --------------------------------------------------------------------------------------
void GhostController::SetTiles(GhostTypes::GhostTypeEnum ghost, int32_t h, int32_t v)
{
  HTile[ghost] = h;
  VTile[ghost] = v;
}

// --------------------------------------------------------------------------------------
// Function: GetTiles
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::GetTiles(GhostTypes::GhostTypeEnum ghost, int32_t& h, int32_t& v)
{
  h = HTile[ghost];
  v = VTile[ghost];
}

// --------------------------------------------------------------------------------------
// Function: SetX
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetX(GhostTypes::GhostTypeEnum ghost, float x)
{
  X[ghost] = x;
  Sprite[ghost]->SetX(X[ghost] - GhostTypes::BorderPadding);
}

// --------------------------------------------------------------------------------------
// Function: GetX
// Notes: None
// --------------------------------------------------------------------------------------
float GhostController::GetX(GhostTypes::GhostTypeEnum ghost) const
{
  return X[ghost];
}

// --------------------------------------------------------------------------------------
// Function: SetY
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetY(GhostTypes::GhostTypeEnum ghost, float y)
{
  Y[ghost] = y;
  Sprite[ghost]->SetY(Y[ghost] - GhostTypes::BorderPadding);
}

// --------------------------------------------------------------------------------------
// Function: GetY
// Notes: None
// --------------------------------------------------------------------------------------
float GhostController::GetY(GhostTypes::GhostTypeEnum ghost) const
{
  return Y[ghost];
}

// --------------------------------------------------------------------------------------
// Function: ResetStartPosition
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::ResetStartPosition(GhostTypes::GhostTypeEnum ghost)
{
  SetState(ghost, ENCASED);

  // Set initial point
  int32_t hTile = 0;
  int32_t vTile = 0;
  switch (ghost)
  {
  case GhostTypes::BLINKY:
    hTile = 12;
    vTile = 14;
    break;
  case GhostTypes::PINKY:
    hTile = 15;
    vTile = 14;
    break;
  case GhostTypes::INKY:
    hTile = 13;
    vTile = 15;
    break;
  case GhostTypes::SUE:
    hTile = 14;
    vTile = 15;
    break;
  }

  float x = 0.0f;
  float y = 0.0f;
  GhostArena::Instance()->TileToPos(hTile, vTile, x, y);

  SetX(ghost, x);
  SetY(ghost, y);
  SetTiles(ghost, hTile, vTile);
}

// --------------------------------------------------------------------------------------
// Function: ResetDestination
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::ResetDestination(GhostTypes::GhostTypeEnum ghost)
{
  TransitionFrame[ghost] = 0;
  DestX[ghost] = 0;
  DestY[ghost] = 0;
}

// --------------------------------------------------------------------------------------
// Function: ResetDestination
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetGhostsVulnerable()
{
  SetVulnerable[GhostTypes::BLINKY] = true;
  SetVulnerable[GhostTypes::PINKY] = true;
  SetVulnerable[GhostTypes::INKY] = true;
  SetVulnerable[GhostTypes::SUE] = true;
  ScoreMultiplier = 1;
}

// --------------------------------------------------------------------------------------
// Function: IsMovementLegal
// Notes: None
// --------------------------------------------------------------------------------------
bool GhostController::IsMovementLegal(GhostTypes::GhostTypeEnum ghost,
                                      GhostTypes::DirectionTypeEnum direction,
                                      int32_t offset)
{
  int32_t hTile = HTile[ghost];
  int32_t vTile = VTile[ghost];
  StateEnum state = CurrentState[ghost];

  // Modify tiles with offset depending on direction;
  switch (direction)
  {
  case GhostTypes::RIGHT:
    hTile += offset;
    break;
  case GhostTypes::LEFT:
    hTile -= offset;
    break;
  case GhostTypes::UP:
    vTile -= offset;
    break;
  case GhostTypes::DOWN:
    vTile += offset;
    break;
  }

  return IsMovementLegal(ghost, direction, hTile, vTile);
}

// --------------------------------------------------------------------------------------
// Function: IsMovementLegal
// Notes: None
// --------------------------------------------------------------------------------------
bool GhostController::IsMovementLegal(GhostTypes::GhostTypeEnum ghost,
                                      GhostTypes::DirectionTypeEnum direction,
                                      int32_t hTile,
                                      int32_t vTile)
{
  StateEnum state = CurrentState[ghost];

  // If the state is entering/exiting and we are moving upwards towards the ghost 
  // box door's, return true. Otherwise invoke the arena's version of this function.
  if ((((StateEnum::ENTERING == state)) ||
       ((StateEnum::EXITING == state))) &&
      ((11 <= vTile) && (13 >= vTile)) &&
      ((13 <= hTile) && (14 >= hTile)))
  {
    return true;
  }

  return GhostArena::Instance()->IsMovementLegal(hTile, vTile, direction);
}

// --------------------------------------------------------------------------------------
// Function: PacmanController
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::UpdateDirection(GhostTypes::GhostTypeEnum ghost)
{
  // If there is a Direction Sequence and the ghost is stuck,
  // empty the sequence and find a new path.
  if ((0 != DirSequence[ghost].size()) &&
      (false == PlayAnimation[ghost]))
  {
    DirSequence[ghost].clear();
  }

  // TODO: CHASING/RETREATING should be able to change directions
  if (0 == DirSequence[ghost].size())
  {
    switch (CurrentState[ghost])
    {
      case ENCASED:
      {
        SetRandDirection(ghost);
      }
      break;
      case ENTERING:
      {
        SetEnterDirection(ghost);
      }
      break;
      case ROAMING:
      {
        SetRandDirection(ghost);
      }
      break;
      case CHASING:
      {
        // Retrieve target tiles
        int32_t tH = 0;
        int32_t tV = 0;
        PacmanController::Instance()->GetTiles(tH, tV);

        // If the distance from ghost to target is extensive, shorten the length.
        int32_t gH = HTile[ghost];
        int32_t gV = VTile[ghost];
        double distance = sqrt(pow(HTile[ghost] - tH, 2) + 
                               pow(VTile[ghost] - tV, 2));
        MaximumSequence[ghost] = (10.0 < distance ? 10 : 32);

        // Retrieve directions
        SetTargetDirection(ghost, tH, tV, true);
      } 
      break;
      case RETREATING:
      {
        int32_t tH = 0;
        int32_t tV = 0;
        PacmanController::Instance()->GetTiles(tH, tV);

        // If the distance between the ghost and player is <= 5, choose
        // a direction away from the player, otherwise move randomly.
        double distance = sqrt(pow(HTile[ghost] - tH, 2) + 
                               pow(VTile[ghost] - tV, 2));
        if (10.0 < distance)
        {
          SetRandDirection(ghost);
        }
        else
        {
          MaximumSequence[ghost] = 16;
          SetTargetDirection(ghost, tH, tV, false);
        }
      }
      break;
      case EXITING:
      {
        MaximumSequence[ghost] = 32;
        SetExitDirection(ghost);
      };
      break;
    }

  }
  
  // Pull a direction request if it needs updating
  if (0 != DirSequence[ghost].size())
  {
    DirRequest[ghost] = DirSequence[ghost].back();
    DirSequence[ghost].pop_back();

    // Trim DirSequence
    if (DirSequence[ghost].size() > MaximumSequence[ghost])
    {
      size_t cnt = DirSequence[ghost].size() - MaximumSequence[ghost] - 1;
      DirSequence[ghost].erase(DirSequence[ghost].begin(), 
                                DirSequence[ghost].begin() + cnt);
    }
  }
}

// --------------------------------------------------------------------------------------
// Function: SetRandDirection
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetRandDirection(GhostTypes::GhostTypeEnum ghost)
{
  int32_t hTile = HTile[ghost];
  int32_t vTile = VTile[ghost];

  // Add all legal directions
  std::vector<GhostTypes::DirectionTypeEnum> directions;

  if (true == PlayAnimation[ghost])
  {
    directions.push_back(Direction[ghost]);
  }

  switch (Direction[ghost])
  {
  case GhostTypes::LEFT:
    directions.push_back(GhostTypes::UP);
    directions.push_back(GhostTypes::DOWN);
    directions.push_back(GhostTypes::RIGHT);
    break;
  case GhostTypes::RIGHT:
    directions.push_back(GhostTypes::UP);
    directions.push_back(GhostTypes::DOWN);
    directions.push_back(GhostTypes::LEFT);
    break;
  case GhostTypes::UP:
    directions.push_back(GhostTypes::LEFT);
    directions.push_back(GhostTypes::RIGHT);
    directions.push_back(GhostTypes::DOWN);
    break;
  case GhostTypes::DOWN:
    directions.push_back(GhostTypes::LEFT);
    directions.push_back(GhostTypes::RIGHT);
    directions.push_back(GhostTypes::UP);
    break;
  }

  while (0 != directions.size())
  {
    uint32_t dirIndex = 0;
    if (2 < directions.size())
    {
      // Avoid opposite direction at random
      dirIndex = rand() % (directions.size() - 1); 
    }

    GhostTypes::DirectionTypeEnum dir = directions[dirIndex];
    if (true == GhostArena::Instance()->IsMovementLegal(hTile, vTile, dir))
    {
      DirSequence[ghost].push_back(dir);
      break; // Found new direction
    }
  
    directions.erase(directions.begin() + dirIndex);
  }
}

// --------------------------------------------------------------------------------------
// Function: SetEnterDirection
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetEnterDirection(GhostTypes::GhostTypeEnum ghost)
{
  // If a ghost has reached a certain two tiles and the state
  // is ENTERING, update the state.
  if (13 > HTile[ghost])
  {
    DirRequest[ghost] = GhostTypes::RIGHT;
    return;
  }
  if (14 < HTile[ghost])
  {
    DirRequest[ghost] = GhostTypes::LEFT;
    return;
  }

  DirRequest[ghost] = GhostTypes::UP;
}

// --------------------------------------------------------------------------------------
// Function: SetExitDirection
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetExitDirection(GhostTypes::GhostTypeEnum ghost)
{
  // Aim towards the ghost box, then append down direction
  SetTargetDirection(ghost, 13, 11, true);
  if (0 != DirSequence[ghost].size())
  {
    std::vector<GhostTypes::DirectionTypeEnum> endDirs = DirSequence[ghost];
    DirSequence[ghost].clear();
    DirSequence[ghost].push_back(GhostTypes::DOWN);
    DirSequence[ghost].push_back(GhostTypes::DOWN);
    DirSequence[ghost].insert(DirSequence[ghost].end(), endDirs.begin(), endDirs.end());
  }
}

// --------------------------------------------------------------------------------------
// Function: SetTargetDirection
// Notes: None.
// --------------------------------------------------------------------------------------
void GhostController::SetTargetDirection(GhostTypes::GhostTypeEnum ghost, 
                                         int32_t targetH, 
                                         int32_t targetV, 
                                         bool towards)
{
  // TODO: write !towards, A*, find shortest path, etc..
  if (true == towards)
  {
    if (EXITING == CurrentState[ghost])
    {
      FindPathAStar(ghost, targetH, targetV, DirSequence[ghost]);
    }
    else
    {
      FindPathAStar(ghost, targetH, targetV, DirSequence[ghost]);
    }
  }
  else
  {
    // Locate a position in the opposite direction of the player. 
    int32_t retreatH = HTile[ghost];
    int32_t retreatV = VTile[ghost];

    if ((targetH > HTile[ghost]) ||
        ((targetH == HTile[ghost]) &&
         (HTile[ghost] < (static_cast<int32_t>(GhostTypes::HTileCount) / 2))))
    {
      for (int32_t h = 1; h <= HTile[ghost]; ++h)
      {
        if (GhostArena::Instance()->IsTileLegal(h, retreatV))
        {
          retreatH = h;
          break;
        }
      }
    }
    else if ((targetH < HTile[ghost]) ||
             ((targetH == HTile[ghost]) &&
              (HTile[ghost] > (static_cast<int32_t>(GhostTypes::HTileCount) / 2))))
    {
      for (int32_t h = GhostTypes::HTileCount - 1; h >= HTile[ghost]; --h)
      {
        if (GhostArena::Instance()->IsTileLegal(h, retreatV))
        {
          retreatH = h;
          break;
        }
      }
    }

    if ((targetV > VTile[ghost]) ||
        ((targetV == VTile[ghost]) &&
         (VTile[ghost] > (static_cast<int32_t>(GhostTypes::VTileCount) / 2))))
    {
      for (int32_t v = 1; v <= VTile[ghost]; ++v)
      {
        if (GhostArena::Instance()->IsTileLegal(retreatH, v))
        {
          retreatV = v;
          break;
        }
      }
    }
    else if ((targetV < VTile[ghost]) ||
             ((targetV == VTile[ghost]) &&
              (VTile[ghost] < (static_cast<int32_t>(GhostTypes::VTileCount) / 2))))
    {
      for (int32_t v = GhostTypes::VTileCount - 1; v >= VTile[ghost]; --v)
      {
        if (GhostArena::Instance()->IsTileLegal(retreatH, v))
        {
          retreatV = v;
          break;
        }
      }
    }

    FindPathAStar(ghost, retreatH, retreatV, DirSequence[ghost]);
  }
}

// --------------------------------------------------------------------------------------
// Function: FindPathBFS
// Notes: None.
// --------------------------------------------------------------------------------------
void GhostController::FindPathBFS(GhostTypes::GhostTypeEnum ghost,
                                  int32_t targetH,
                                  int32_t targetV,
                                  std::vector<GhostTypes::DirectionTypeEnum>& path)
{
  path.clear();

  // Retrieve ghost's tiles
  int32_t gTileH = HTile[ghost];
  int32_t gTileV = VTile[ghost];

  std::vector<uint16_t> usedTiles;
  usedTiles.push_back((gTileH << 8) | gTileV);

  // Free the list if path not found.
  if (false == FindPathBFS(ghost, targetH, targetV, path, usedTiles))
  {
    path.clear();
  }
}

// --------------------------------------------------------------------------------------
// Function: FindPathDFS
// Notes: TODO:
// --------------------------------------------------------------------------------------
void GhostController::FindPathDFS(GhostTypes::GhostTypeEnum ghost,
                                  int32_t targetH,
                                  int32_t targetV,
                                  std::vector<GhostTypes::DirectionTypeEnum>& path)
{
  path.clear();

  // Retrieve ghost's tiles
  int32_t gTileH = HTile[ghost];
  int32_t gTileV = VTile[ghost];

  std::vector<uint16_t> usedTiles;
  usedTiles.push_back((gTileH << 8) | gTileV);

  // Free the list if path not found.
  if (false == FindPathDFS(ghost, targetH, targetV, path, usedTiles))
  {
    path.clear();
  }
}

// --------------------------------------------------------------------------------------
// Function: FindPathAStar
// Notes: None.
// --------------------------------------------------------------------------------------
void GhostController::FindPathAStar(GhostTypes::GhostTypeEnum ghost,
                                    int32_t targetH,
                                    int32_t targetV,
                                    std::vector<GhostTypes::DirectionTypeEnum>& path)
{
  path.clear();

  // Define mini search struct
  struct SearchTile
  {
    int32_t h;
    int32_t v;
    int32_t pH;
    int32_t pV;
    int32_t tH;
    int32_t tV;
    float gCost;
    float hCost;
    float fCost;
    GhostTypes::DirectionTypeEnum pDir;

    SearchTile() :
      h(0), v(0), pH(-1), pV(-1), fCost(FLT_MAX), hCost(FLT_MAX), gCost(FLT_MAX)
    {}
  };

  // Initialize Search Map and Closed List
  bool* closedList = new bool[GhostTypes::VTileCount * GhostTypes::HTileCount];
  memset(closedList, 
         false, 
         sizeof(bool) * GhostTypes::VTileCount* GhostTypes::HTileCount);

  SearchTile* searchMap = 
    new SearchTile[GhostTypes::VTileCount * GhostTypes::HTileCount];
  for (int32_t v = 0; v < static_cast<int32_t>(GhostTypes::VTileCount); ++v)
  {
    for (int32_t h = 0; h < static_cast<int32_t>(GhostTypes::HTileCount); ++h)
    {
      searchMap[(v * GhostTypes::HTileCount) + h].h = h;
      searchMap[(v * GhostTypes::HTileCount) + h].v = v;
    }
  }

  // Initialize starting list
  int32_t currH = HTile[ghost];
  int32_t currV = VTile[ghost];
  uint32_t currIndex = GhostTypes::TileIndex(currH, currV);
  searchMap[currIndex].fCost = 0.0f;
  searchMap[currIndex].hCost = 0.0f;
  searchMap[currIndex].gCost = 0.0f;
  searchMap[currIndex].pH = currH;
  searchMap[currIndex].pV = currV;

  // Begin search 
  bool found = false;
  std::vector<SearchTile> openList;
  openList.emplace_back(searchMap[currIndex]);

  while ((false == openList.empty()) &&
         (openList.size() < GhostTypes::TotalTiles))
  {
    SearchTile tile;
    do
    {
      float temp = FLT_MAX;
      std::vector<SearchTile>::iterator itrNode = openList.end();
      for (std::vector<SearchTile>::iterator itr = openList.begin();
           itr != openList.end();
           ++itr)
      {
        SearchTile currTile = *itr;
        if (currTile.fCost < temp)
        {
          temp = currTile.fCost;
          itrNode = itr;
        }
      }

      // No path found, return. We might be at the destination already!
      if (openList.end() == itrNode)
      {
        return;
      }

      tile = *itrNode;
      openList.erase(itrNode);
    } while (false == GhostArena::Instance()->IsTileLegal(tile.h, tile.v));

    currH = tile.h;
    currV = tile.v;
    currIndex = GhostTypes::TileIndex(currH, currV);
    closedList[currIndex] = true;

    for (uint32_t dir = GhostTypes::RIGHT; dir <= GhostTypes::DOWN; ++dir)
    {
      float newGCost = 0.0f;
      float newHCost = 0.0f;
      float newFCost = 0.0f;

      int32_t nextH = currH;
      int32_t nextV = currV;
      switch (dir)
      {
      case GhostTypes::RIGHT:
        ++nextH;
        break;
      case GhostTypes::LEFT:
        --nextH;
        break;
      case GhostTypes::UP:
        --nextV;
        break;
      case GhostTypes::DOWN:
        ++nextV;
        break;
      }

      if (true == GhostArena::Instance()->IsTileLegal(nextH, nextV))
      {
        uint32_t nextIndex = GhostTypes::TileIndex(nextH, nextV);

        // Check for destination
        if ((targetH == nextH) &&
            (targetV == nextV))
        {
          // Add final direction
          path.push_back(static_cast<GhostTypes::DirectionTypeEnum>(dir));

          // Add rest of path
          while (((HTile[ghost] != searchMap[currIndex].pH) ||
                  (VTile[ghost] != searchMap[currIndex].pV)) &&
                  (-1 != searchMap[currIndex].h) &&
                  (-1 != searchMap[currIndex].v))
          {
            path.push_back(searchMap[currIndex].pDir);
            currH = searchMap[currIndex].pH;
            currV = searchMap[currIndex].pV;
            currIndex = GhostTypes::TileIndex(currH, currV);
          }
    
          path.push_back(searchMap[currIndex].pDir);
          delete[] closedList;
          delete[] searchMap;
          return;
        }
        else if (false == closedList[nextIndex])
        {
          newGCost = static_cast<float>(tile.gCost + 1.0f);
          newHCost = static_cast<float>(sqrt(pow(currH - targetH, 2) + 
                                        pow(currV - targetV, 2)));
          newFCost = static_cast<float>(newGCost + newHCost);

          // Check if this path is better than current
          if ((FLT_MAX == searchMap[nextIndex].fCost) ||
              (newFCost < searchMap[nextIndex].fCost))
          {
            searchMap[nextIndex].gCost = newGCost;
            searchMap[nextIndex].hCost = newHCost;
            searchMap[nextIndex].fCost = newFCost;
            searchMap[nextIndex].pH = currH;
            searchMap[nextIndex].pV = currV;
            searchMap[nextIndex].pDir = static_cast<GhostTypes::DirectionTypeEnum>(dir);
            openList.emplace_back(searchMap[nextIndex]);
          }
        }
      }
    }
  }

  delete[] closedList;
  delete[] searchMap;
}

// --------------------------------------------------------------------------------------
// Function: FindPathBFS
// Notes: None.
// --------------------------------------------------------------------------------------
bool GhostController::FindPathBFS(GhostTypes::GhostTypeEnum ghost,
                                  int32_t targetH,
                                  int32_t targetV,
                                  std::vector<GhostTypes::DirectionTypeEnum>& path,
                                  std::vector<uint16_t>& usedTiles)
{
  // Retrieve the ghost's current tiles
  int32_t gTileH = usedTiles.back() >> 8;;
  int32_t gTileV = usedTiles.back() & 0xFF;

  // Construct a direction list to search through. It should be sorted by
  // distance pointing towards the destination.
  int32_t hTileDelta = targetH - gTileH;
  int32_t vTileDelta = targetV - gTileV;
  bool dirFlags[GhostTypes::DOWN + 1] = { false, false, false, false };

  std::vector<GhostTypes::DirectionTypeEnum> directions;
  if (pow(hTileDelta, 2) > pow(vTileDelta, 2)) // Horizontally further
  {
    if (hTileDelta < 0) // Turn left
    {
      dirFlags[GhostTypes::LEFT] = true;
      directions.push_back(GhostTypes::LEFT);
    }
    else if (hTileDelta > 0) // Turn right
    {
      dirFlags[GhostTypes::RIGHT] = true;
      directions.push_back(GhostTypes::RIGHT);
    }
  }
  else // Vertically further
  {
    if (vTileDelta < 0) // Go up
    {
      dirFlags[GhostTypes::UP] = true;
      directions.push_back(GhostTypes::UP);
    }
    else if (vTileDelta > 0) // Go down
    {
      dirFlags[GhostTypes::DOWN] = true;
      directions.push_back(GhostTypes::DOWN);
    }
  }
  
  // Add remaining directions
  for (uint32_t dir = GhostTypes::RIGHT; dir <= GhostTypes::DOWN; ++dir)
  {
    if (false == dirFlags[dir])
    {
      directions.push_back(static_cast<GhostTypes::DirectionTypeEnum>(dir));
    }
  }

  // Check all adjacent platforms for the target first, then proceed to search 
  // through them recursively.
  for (uint32_t i = 0; i < directions.size(); ++i)
  {
    int32_t deltaH = gTileH;
    int32_t deltaV = gTileV;
    switch (directions[i])
    {
    case GhostTypes::RIGHT:
      ++deltaH;
      break;
    case GhostTypes::LEFT:
      --deltaH;
      break;
    case GhostTypes::UP:
      --deltaV;
      break;
    case GhostTypes::DOWN:
      ++deltaV;
      break;
    }

    // If we reached the destination, return true.
    if ((targetH == deltaH) &&
        (targetV == deltaV))
    {
      path.push_back(directions[i]);
      return true;
    }
  }

  // Recursively search all available paths.
  for (uint32_t i = 0; i < directions.size(); ++i)
  {
    // If the move is legal, recursively search in that direction if the target
    // tile is not in usedTiles.
    if (true == IsMovementLegal(ghost, directions[i], gTileH, gTileV))
    {
      int32_t deltaH = gTileH;
      int32_t deltaV = gTileV;
      switch (directions[i])
      {
      case GhostTypes::RIGHT:
        ++deltaH;
        break;
      case GhostTypes::LEFT:
        --deltaH;
        break;
      case GhostTypes::UP:
        --deltaV;
        break;
      case GhostTypes::DOWN:
        ++deltaV;
        break;
      }

      bool tileUsed = false;
      for (uint16_t tile : usedTiles)
      {
        if (tile == ((deltaH << 8) | deltaV))
        {
          tileUsed = true;
          break;
        }
      }

      // If the tile is not used recursively call self.
      if (false == tileUsed)
      {
        // Add the next tile set to the used tiles list.
        usedTiles.push_back((deltaH << 8) | deltaV);

        if (true == FindPathBFS(ghost, targetH, targetV, path, usedTiles))
        {
          // This means we eventually found a path, add the direction.
          path.push_back(directions[i]);
          return true;
        }
      }
    }
  }

  // None of the directions were legal or we reached a used tile.
  return false; 
}

// --------------------------------------------------------------------------------------
// Function: FindPathDFS
// Notes: TODO:
// --------------------------------------------------------------------------------------
bool GhostController::FindPathDFS(GhostTypes::GhostTypeEnum ghost,
                                  int32_t targetH,
                                  int32_t targetV,
                                  std::vector<GhostTypes::DirectionTypeEnum>& path,
                                  std::vector<uint16_t>& usedTiles)
{
  // Retrieve the ghost's current tiles
  int32_t gTileH = usedTiles.back() >> 8;;
  int32_t gTileV = usedTiles.back() & 0xFF;

  // If we reached the destination, return true.
  if ((targetH == gTileH) &&
      (targetV == gTileV))
  {
    return true;
  }

  // Construct a direction list to search through. It should be sorted by
  // distance pointing towards the destination.
  int32_t hTileDelta = targetH - gTileH;
  int32_t vTileDelta = targetV - gTileV;
  bool dirFlags[GhostTypes::DOWN + 1] = { false, false, false, false };

  std::vector<GhostTypes::DirectionTypeEnum> directions;
  if (pow(hTileDelta, 2) > pow(vTileDelta, 2)) // Horizontally further
  {
    if (hTileDelta < 0) // Turn left
    {
      dirFlags[GhostTypes::LEFT] = true;
      directions.push_back(GhostTypes::LEFT);
    }
    else if (hTileDelta > 0) // Turn right
    {
      dirFlags[GhostTypes::RIGHT] = true;
      directions.push_back(GhostTypes::RIGHT);
    }
  }
  else // Vertically further
  {
    if (vTileDelta < 0) // Go up
    {
      dirFlags[GhostTypes::UP] = true;
      directions.push_back(GhostTypes::UP);
    }
    else if (vTileDelta > 0) // Go down
    {
      dirFlags[GhostTypes::DOWN] = true;
      directions.push_back(GhostTypes::DOWN);
    }
  }

  // Add remaining directions
  for (uint32_t dir = GhostTypes::RIGHT; dir <= GhostTypes::DOWN; ++dir)
  {
    if (false == dirFlags[dir])
    {
      directions.push_back(static_cast<GhostTypes::DirectionTypeEnum>(dir));
    }
  }

  // Try moving in all directions as long as we don't reach a used tile.
  for (uint32_t i = 0; i < directions.size(); ++i)
  {
    // If the move is legal, recursively search in that direction if the target
    // tile is not in usedTiles.
    if (true == IsMovementLegal(ghost, directions[i], gTileH, gTileV))
    {
      int32_t deltaH = gTileH;
      int32_t deltaV = gTileV;
      switch (directions[i])
      {
      case GhostTypes::RIGHT:
        ++deltaH;
        break;
      case GhostTypes::LEFT:
        --deltaH;
        break;
      case GhostTypes::UP:
        --deltaV;
        break;
      case GhostTypes::DOWN:
        ++deltaV;
        break;
      }

      bool tileUsed = false;
      for (uint16_t tile : usedTiles)
      {
        if (tile == ((deltaH << 8) | deltaV))
        {
          tileUsed = true;
          break;
        }
      }

      // If the tile is not used recursively call self.
      if (false == tileUsed)
      {
        // Add the next tile set to the used tiles list.
        usedTiles.push_back((deltaH << 8) | deltaV);

        if (true == FindPathDFS(ghost, targetH, targetV, path, usedTiles))
        {
          // This means we eventually found a path, add the direction.
          path.push_back(directions[i]);
          return true;
        }
      }
    }
  }

  // None of the directions were legal or we reached a used tile.
  return false; 
}

// --------------------------------------------------------------------------------------
// Function: SetState
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::SetState(GhostTypes::GhostTypeEnum ghost, StateEnum state)
{
  // If the game is over, we need to set the state to roaming.
  if (true == GhostSchool::Instance()->IsGameOver())
  {
    CurrentState[ghost] = ROAMING;
    return;
  }

  // If the current state is RETREATING, and the next state is not,
  // stop RETREATING audio if no ghosts are left.
  if ((RETREATING == CurrentState[ghost]) &&
      (RETREATING != state))
  {
    bool stopSound = true;
    for (uint32_t g = GhostTypes::BLINKY; g <= GhostTypes::SUE; ++g)
    {
      if ((ghost != g) &&
          (RETREATING == CurrentState[g]))
      {
        stopSound = false;
        break;
      }
    }

    if (true == stopSound)
    {
      SoundEngine::Instance()->Stop(GhostTypes::AUDIO_VULN_ID);
    }
  }

  // If the new state is vulnerable, do not set it if the current state
  // is ENCASED, ENTERING, or EXITING.
  uint32_t adjustment = GhostArena::Instance()->GetCurrentLevel();
  uint32_t speedAdj = GhostSchool::Instance()->GetGhostSpeedAdj();
  switch (state)
  {
  case ENCASED:
    TransitionMax[ghost] = GhostTypes::BaseTransition;
    LastState[ghost] = CurrentState[ghost];
    RemainingState[ghost] = 40 + (ghost * 5) - adjustment;
    break;
  case ENTERING:
    TransitionMax[ghost] = GhostTypes::BaseTransition;
    LastState[ghost] = CurrentState[ghost];
    break;
  case ROAMING:
    TransitionMax[ghost] = GhostTypes::BaseTransition - speedAdj;
    LastState[ghost] = CurrentState[ghost];
    RemainingState[ghost] = 40 - adjustment;
  break;
  case CHASING:
    TransitionMax[ghost] = GhostTypes::BaseTransition - speedAdj;
    LastState[ghost] = CurrentState[ghost];
    RemainingState[ghost] = 200 + (10 * adjustment);
    break;
  case RETREATING:
    if ((ENCASED == CurrentState[ghost]) ||
        (ENTERING == CurrentState[ghost]) ||
        (EXITING == CurrentState[ghost]))
    {
      state = CurrentState[ghost];
      break;
    }

    TransitionMax[ghost] = (GhostTypes::BaseTransition * 2) - speedAdj;
    RemainingState[ghost] = 40 - adjustment;
    if (RETREATING != CurrentState[ghost])
    {
      LastState[ghost] = CurrentState[ghost];
    }

    break;
  case EXITING:
    TransitionMax[ghost] = GhostTypes::BaseTransition;
    LastState[ghost] = CurrentState[ghost];
    break;
  }

  CurrentState[ghost] = state;

  // Shorten our direction sequence.
  if (1 < DirSequence[ghost].size())
  {
    DirSequence[ghost].erase(DirSequence[ghost].begin(), DirSequence[ghost].end() - 1);
  }
}

// --------------------------------------------------------------------------------------
// Function: UpdateState
// Notes: None
// --------------------------------------------------------------------------------------
void GhostController::UpdateState(GhostTypes::GhostTypeEnum ghost)
{
  // Update for every case except for when transitioning to the next state.
  bool shouldUpdate = true;

  // If we are setting to vulnerable, do so now.
  if (SetVulnerable[ghost])
  {
    SetVulnerable[ghost] = false;
    SetState(ghost, RETREATING);
    return;
  }

  // Process remaining state or state change
  switch (CurrentState[ghost])
  {
    case ENCASED:
    {
      --RemainingState[ghost];
      if (0 == RemainingState[ghost])
      {
        SetState(ghost, ENTERING);
        shouldUpdate = false;
      }
    }
    break;
    case ENTERING:
    {
      if (((13 == HTile[ghost]) ||
           (14 == HTile[ghost])) &&
          (11 == VTile[ghost]))
      {
        SetState(ghost, ROAMING);
        shouldUpdate = false;
      }
    }
    break;
    case ROAMING:
    {
      --RemainingState[ghost];
      if (0 == RemainingState[ghost])
      {
        // When updating from ROAMING to CHASING, synchronize
        // all ghosts capable of chasing. The higher the level,
        // the more ghosts involved.
        uint32_t maxGhost = GhostTypes::SUE;
        uint32_t currLevel = GhostArena::Instance()->GetCurrentLevel();

        // When reaching arena 2, INKY begins to chase.
        if (2 > currLevel)
        {
          --maxGhost;
        }
        // When reaching arena 3, all ghosts chase.
        if (5 > currLevel)
        {
          --maxGhost;
        }

        for (uint32_t g = GhostTypes::BLINKY; g <= maxGhost; ++g)
        {
          if ((ROAMING == CurrentState[g]) ||
              (CHASING == CurrentState[g]) ||
              (RETREATING == CurrentState[g]))
          {
            SetState(static_cast<GhostTypes::GhostTypeEnum>(g), CHASING);
          }
        }

        shouldUpdate = false;
      }
    }
    break;
    case CHASING:
    {
      --RemainingState[ghost];
      if (0 == RemainingState[ghost])
      {
        SetState(ghost, ROAMING);
        shouldUpdate = false;
      }
    }
    break;
    case RETREATING:
    {
      --RemainingState[ghost];
      if (0 == RemainingState[ghost])
      {
        // When updating from RETREATING, synchronize
        // all ghosts currently still in retreatment.
        for (int32_t g = GhostTypes::BLINKY; g <= GhostTypes::SUE; ++g)
        {
          if (ghost != g)
          {
            if (RETREATING == CurrentState[g])
            {
              RemainingState[g] = 1;
            }
          }
        }

        // If we run out of the retreat state, set it to the last state.
        SetState(ghost, LastState[ghost]);
        shouldUpdate = false;

        // Reset Score multiplier
        ScoreMultiplier = 1;
      }
    }
    break;
    case EXITING:
    {
      if (((13 == HTile[ghost]) ||
           (14 == HTile[ghost])) &&
          (13 == VTile[ghost]))
      {
        SetState(ghost, ENCASED);
        shouldUpdate = false;
      }
    }
    break;
  }

  // We only update the direction if an immedate state change was not made.
  if (true == shouldUpdate)
  {
    UpdateDirection(ghost);
  }
}

// --------------------------------------------------------------------------------------
// Function: PacmanController
// Notes: None
// --------------------------------------------------------------------------------------
GhostController::GhostController() :
  X{0},
  Y{0},
  DestX{0, 0, 0, 0},
  DestY{0, 0, 0, 0},
  DeltaX{0, 0, 0, 0},
  DeltaY{0, 0, 0, 0},
  HTile{0, 0, 0, 0},
  VTile{0, 0, 0, 0},
  SkipUpdate(false),
  Sprite{nullptr, nullptr, nullptr, nullptr},
  TransitionMax{8, 8, 8, 8},
  TransitionFrame{0, 0, 0, 0},
  PlayAnimation{true, true, true, true},
  LastTile{GhostTypes::NOTHING, 
           GhostTypes::NOTHING,
           GhostTypes::NOTHING,
           GhostTypes::NOTHING},
  Direction{GhostTypes::UP, 
            GhostTypes::UP, 
            GhostTypes::UP, 
            GhostTypes::UP},
  DirRequest{GhostTypes::NO_DIRECTION, 
             GhostTypes::NO_DIRECTION, 
             GhostTypes::NO_DIRECTION, 
             GhostTypes::NO_DIRECTION},
  MaximumSequence{32, 32, 32, 32},
  ScoreMultiplier(1),
  SetVulnerable{false, false, false, false},
  CurrentState{ENCASED, ENCASED, ENCASED, ENCASED},
  LastState{ENCASED, ENCASED, ENCASED, ENCASED},
  RemainingState{100, 100, 100, 100},
  AnimationOffset{0, 2, 4, 6, 8, 12},
  AnimationTick{0, 0, 0, 0}
{
}

// --------------------------------------------------------------------------------------
// Function: ~PacmanController
// Notes: None
// --------------------------------------------------------------------------------------
GhostController::~GhostController()
{

}