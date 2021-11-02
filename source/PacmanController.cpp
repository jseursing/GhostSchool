#include "GhostArena.h"
#include "GhostController.h"
#include "GhostSchool.h"
#include "OGLSprite.h"
#include "OGLSpriteMgr.h"
#include "PacmanController.h"
#include "SoundEngine.h"


// --------------------------------------------------------------------------------------
// Function: Instance
// Notes: None
// --------------------------------------------------------------------------------------
PacmanController* PacmanController::Instance(uint32_t programId)
{
  static PacmanController instance;
  if (nullptr == instance.Sprite)
  {
    instance.Sprite = 
      OGLSpriteMgr::Instance()->RegisterSprite(programId,
                                               "playerTexture",
                                               GhostTypes::Sprite_Player, 
                                               3, 
                                               4);
    instance.Sprite->SetSize(GhostTypes::PlayerSize, 
                             GhostTypes::PlayerSize);
    instance.ResetStartPosition();
  }

  return &instance;
}

// --------------------------------------------------------------------------------------
// Function: LockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::LockUpdate()
{
  SkipUpdate = true;
}

// --------------------------------------------------------------------------------------
// Function: UnlockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::UnlockUpdate()
{
  SkipUpdate = false;
}

// --------------------------------------------------------------------------------------
// Function: Instance
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::Update(uint32_t tick)
{
  // Update animation at a high frequency
  if (false == SkipUpdate)
  {
    // If death animation is active, continue to rotate the 
    // player until the sound is done.
    if (0 != DeathTicks)
    {
      --DeathTicks;

      if (0 == DeathTicks)
      {
        // Stop sound
        SoundEngine::Instance()->Stop(GhostTypes::AUDIO_DEATH_ID);
        
        // Adjust lives...
        GhostSchool::Instance()->ProcessPlayerDeath();

        // Unlock ghosts
        GhostController::Instance()->UnlockUpdate();
      }
      else 
      {
        if (false == SoundEngine::Instance()->IsActive(GhostTypes::AUDIO_DEATH_ID))
        {
          // Reset degree
          Sprite->SetDegree(0);

          // Reset start position
          ResetStartPosition();
          ResetDestination();
        }
        else
        {
          uint32_t degree = DeathTicks * 15;
          Sprite->SetDegree(degree % 360);
        }
      }
  
      return;
    }

    if (0 == (tick % GhostTypes::AnimationFreq))
    {
      if (true == PlayAnimation)
      {
        // Animate Pacman
        uint32_t activeFrame = (Direction * AnimationCount) +
                               (++AnimationTick % AnimationCount);
        Sprite->SetActiveFrame(activeFrame);
      }
    }

    {
      // Update position based on direction only if it is legal.
      int32_t hTile = 0;
      int32_t vTile = 0;
      GetTiles(hTile, vTile);

      // Only update direction if legal and pixels line up perfectly.
      if ((GhostTypes::NO_DIRECTION != DirRequest) &&
          (0 == TransitionFrame))
      {
        if (true == GhostArena::Instance()->IsMovementLegal(hTile, vTile, DirRequest))
        {
          Direction = DirRequest;
          DirRequest = GhostTypes::NO_DIRECTION;
        }
      }

      if (GhostTypes::NO_DIRECTION != Direction)
      {   
        int32_t hTileD = 0;
        int32_t vTileD = 0;

        PlayAnimation = (0 != TransitionFrame) |
                        (GhostArena::Instance()->IsMovementLegal(hTile, vTile, Direction));
        if (true == PlayAnimation)
        {
          switch (Direction)
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

          // TODO: Make player actually exit view and slow down.
          if ((GhostTypes::PIPELINE == 
               GhostArena::Instance()->GetTileType(hTileD, vTileD)) &&
              (GhostTypes::PIPELINE != LastTile))
          {          
            TransitionFrame = 0;
            LastTile = GhostTypes::PIPELINE;
            hTileD = (0 == hTileD ? GhostTypes::HTileCount - 1 : 0);
          
            float currX = 0;
            float currY = 0;
            GhostArena::Instance()->TileToPos(hTileD, vTileD, currX, currY);

            SetX(currX);
            SetY(currY);
            SetTiles(hTileD, vTileD);

            return;
          }

          if (0 == TransitionFrame)
          {
            ++TransitionFrame;

            // Adjust speed
            TransitionMax = GhostTypes::BaseTransition - 
                            GhostSchool::Instance()->GetPlayerSpeedAdj();

            // Update position
            GhostArena::Instance()->TileToPos(hTileD, vTileD, DestX, DestY);
            LastTile = GhostArena::Instance()->GetTileType(hTileD, vTileD);
            DeltaX = (DestX - GetX()) / TransitionMax;
            DeltaY = (DestY - GetY()) / TransitionMax;

            SetTiles(hTileD, vTileD);
            SetX((GetX() + DeltaX));
            SetY((GetY() + DeltaY));
          }
          else if (TransitionMax == TransitionFrame)
          {
            TransitionFrame = 0;
            SetX(DestX);
            SetY(DestY);
          }
          else
          {
            ++TransitionFrame;
            SetX((GetX() + DeltaX));
            SetY((GetY() + DeltaY));
          }
        }
      }
    }
  }
}

// --------------------------------------------------------------------------------------
// Function: ProcessKey
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::ProcessKey(GhostTypes::DirectionTypeEnum direction)
{
  DirRequest = direction;
}

// --------------------------------------------------------------------------------------
// Function: SetTiles
// Notes: Should only be called to set exact position.
// --------------------------------------------------------------------------------------
void PacmanController::SetTiles(int32_t h, int32_t v)
{
  LastHTile = HTile;
  LastVTile = VTile;
  HTile = h;
  VTile = v;
}

// --------------------------------------------------------------------------------------
// Function: GetTiles
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::GetTiles(int32_t& h, int32_t& v)
{
  h = HTile;
  v = VTile;
}

// --------------------------------------------------------------------------------------
// Function: GetLastTiles
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::GetLastTiles(int32_t& h, int32_t& v)
{
  h = LastHTile;
  v = LastVTile;
}


// --------------------------------------------------------------------------------------
// Function: SetX
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::SetX(float x)
{
  X = x;
  Sprite->SetX(X - GhostTypes::BorderPadding);
}

// --------------------------------------------------------------------------------------
// Function: GetX
// Notes: None
// --------------------------------------------------------------------------------------
float PacmanController::GetX() const
{
  return X;
}

// --------------------------------------------------------------------------------------
// Function: SetY
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::SetY(float y)
{
  Y = y; 
  Sprite->SetY(Y - GhostTypes::BorderPadding);
}

// --------------------------------------------------------------------------------------
// Function: GetY
// Notes: None
// --------------------------------------------------------------------------------------
float PacmanController::GetY() const
{
  return Y;
}

// --------------------------------------------------------------------------------------
// Function: ResetDestination
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::SetDirection(GhostTypes::DirectionTypeEnum direction)
{
  Direction = direction;
}

// --------------------------------------------------------------------------------------
// Function: PacmanController
// Notes: None
// --------------------------------------------------------------------------------------
GhostTypes::DirectionTypeEnum PacmanController::GetDirection() const
{
  return Direction;
}

// --------------------------------------------------------------------------------------
// Function: InitiateDeathSequence
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::InitiateDeathSequence()
{
  DeathTicks = 250;
  AnimationTick = 0;
  SoundEngine::Instance()->Play(GhostTypes::AUDIO_DEATH_ID);
}

// --------------------------------------------------------------------------------------
// Function: ResetStartPosition
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::ResetStartPosition()
{
  float startX = 0.0f;
  float startY = 0.0f;
  GhostArena::Instance()->TileToPos(14, 23, startX, startY);

  SetX(startX);
  SetY(startY);
  SetTiles(14, 23);
  SetDirection(GhostTypes::LEFT);
}

// --------------------------------------------------------------------------------------
// Function: ResetDestination
// Notes: None
// --------------------------------------------------------------------------------------
void PacmanController::ResetDestination()
{
  TransitionFrame = 0;
  DestX = 0;
  DestY = 0;
}

// --------------------------------------------------------------------------------------
// Function: PacmanController
// Notes: None
// --------------------------------------------------------------------------------------
PacmanController::PacmanController() :
  X(0),
  Y(0),
  DestX(0),
  DestY(0),
  DeltaX(0),
  DeltaY(0),
  HTile(0),
  VTile(0),
  SkipUpdate(false),
  Sprite(nullptr),
  TransitionMax(8),
  TransitionFrame(0),
  PlayAnimation(false),
  DeathTicks(0),
  LastTile(GhostTypes::NOTHING),
  Direction(GhostTypes::LEFT),
  DirRequest(GhostTypes::NO_DIRECTION),
  AnimationTick(0)
{
}

// --------------------------------------------------------------------------------------
// Function: ~PacmanController
// Notes: None
// --------------------------------------------------------------------------------------
PacmanController::~PacmanController()
{

}