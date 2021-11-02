#include "FruitController.h"
#include "GhostArena.h"
#include "GhostSchool.h"
#include "OGLSpriteMgr.h"
#include "OGLSprite.h"
#include "PacmanController.h"
#include "SoundEngine.h"

// --------------------------------------------------------------------------------------
// Function: Instance
// Notes: None
// --------------------------------------------------------------------------------------
FruitController* FruitController::Instance(uint32_t programId)
{
  static FruitController instance;
  if (nullptr == instance.Sprite)
  {
    instance.Sprite = 
      OGLSpriteMgr::Instance()->RegisterSprite(programId,
                                               "fruitTexture",
                                               GhostTypes::Sprite_Hud, 
                                               8, 
                                               1);
    instance.Sprite->SetSize(GhostTypes::PlayerSize, 
                             GhostTypes::PlayerSize);
    instance.Sprite->SetActiveFrame(instance.FruitId);
    instance.Sprite->DisableRender();
  }

  return &instance;
}

// --------------------------------------------------------------------------------------
// Function: LockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::LockUpdate()
{
  // Lock out updates
  SkipUpdate = true;
  
  // Disable rendering
  Sprite->DisableRender();
}

// --------------------------------------------------------------------------------------
// Function: UnlockUpdate
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::UnlockUpdate()
{
  // Find the bottom left pipe and set the direction.
  srand(static_cast<uint32_t>(time(0)));

  bool enterTop = 0 == rand() % 2;
  int32_t vTile = (true == enterTop ? 1 : GhostTypes::VTileCount - 1);
  if (true == enterTop)
  {
    for (; vTile < static_cast<int32_t>(GhostTypes::VTileCount); ++vTile)
    {
      if (GhostTypes::PIPELINE == GhostArena::Instance()->GetTileType(0, vTile))
      {
        break;
      }
    }
  }
  else
  {
    for (; vTile >= 1; --vTile)
    {
      if (GhostTypes::PIPELINE == GhostArena::Instance()->GetTileType(0, vTile))
      {
        break;
      }
    }
  }

  float x = 0;
  float y = 0;
  GhostArena::Instance()->TileToPos(1, vTile, x, y);

  SetX(x);
  SetY(y);
  SetTiles(1, vTile);
  TransitionFrame = 0; // Reset movement

  // Enable updates
  SkipUpdate = false;

  // This function will also control rendering
  Sprite->EnableRender();
}

// --------------------------------------------------------------------------------------
// Function: IsVisible
// Notes: None
// --------------------------------------------------------------------------------------
bool FruitController::IsVisible() const
{
  return (false == SkipUpdate);
}

// --------------------------------------------------------------------------------------
// Function: SetExitDirection
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::SetExitDirection()
{
  // Find the bottom left pipe and set the direction.
  bool exitTop = 0 == rand() % 2;
  int32_t vTile = (true == exitTop ? 1 : GhostTypes::VTileCount - 1);
  if (true == exitTop)
  {
    for (; vTile > static_cast<int32_t>(GhostTypes::VTileCount); ++vTile)
    {
      if (GhostTypes::PIPELINE == 
          GhostArena::Instance()->GetTileType(GhostTypes::HTileCount - 1, vTile))
      {
        break;
      }
    }
  }
  else
  {
    for (; vTile >= 1; --vTile)
    {
      if (GhostTypes::PIPELINE == 
          GhostArena::Instance()->GetTileType(GhostTypes::HTileCount - 1, vTile))
      {
        break;
      }
    }
  }

  // Set Direction Sequence
  FindPathAStar(GhostTypes::HTileCount - 1, vTile, DirSequence);
}

// --------------------------------------------------------------------------------------
// Function: SetFruit
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::SetFruit(uint32_t id)
{
  if (id < GhostTypes::MaxFruits)
  {
    FruitId = id;
  }
  else
  {
    FruitId = GhostTypes::MaxFruits - 1;
  }

  Sprite->SetActiveFrame(FruitId + 1);
}

// --------------------------------------------------------------------------------------
// Function: Update
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::Update(uint32_t tick)
{
  // Update animation at a high frequency
  if (false == SkipUpdate)
  {
    // Update position based on direction only if it is legal.
    int32_t hTile = 0;
    int32_t vTile = 0;
    GetTiles(hTile, vTile);

    // Check to see if player has eaten this.
    int32_t pTileH = 0;
    int32_t pTileV = 0;
    PacmanController::Instance()->GetTiles(pTileH, pTileV);

    if ((hTile == pTileH) &&
        (vTile == pTileV))
    {
      // Add points, play sound, and lock updates.
      uint32_t points = 0;
      switch (FruitId)
      {
      case 0:
        points = 100;
        break;
      case 1:
        points = 200;
        break;
      case 2:
        points = 500;
        break;
      case 3:
        points = 700;
        break;
      case 4:
        points = 1000;
        break;
      case 5:
        points = 2000;
        break;
      case 6:
        points = 5000;
        break;
      }

      GhostSchool::Instance()->IncreasePoints(points);
      SoundEngine::Instance()->Play(GhostTypes::AUDIO_EATFRUIT_ID, 1.0f);
      GhostSchool::Instance()->DisplayScoreSprite(GetX(), GetY(), true, FruitId);

      LockUpdate();
      return;
    }

    // TODO: Add directional changes from GhostController.
    // Spawn at bottom-left pipe, move randomly, exit out bottom-right pipe.
    if (0 == TransitionFrame)
    {
      if (0 == DirSequence.size())
      {
        SetRandomDirection();
      }
      else
      {
        DirRequest = DirSequence.back();
        DirSequence.pop_back();
      }
    }

    // Only update direction if legal and pixels line up perfectly.
    if ((NO_DIRECTION != DirRequest) &&
        (0 == TransitionFrame))
    {
      if (true == GhostArena::Instance()->IsMovementLegal(hTile, vTile, DirRequest))
      {
        Direction = DirRequest;
        DirRequest = NO_DIRECTION;
      }
    }

    if (NO_DIRECTION != Direction)
    {
      int32_t hTileD = 0;
      int32_t vTileD = 0;

      PlayAnimation = (0 != TransitionFrame) |
                      (GhostArena::Instance()->IsMovementLegal(hTile, vTile, Direction));
      if (true == PlayAnimation)
      {
        switch (Direction)
        {
        case LEFT:
          hTileD = hTile - GhostTypes::TileDelta;
          vTileD = vTile;
          break;
        case UP:
          vTileD = vTile - GhostTypes::TileDelta;
          hTileD = hTile;
          break;
        case RIGHT:
          hTileD = hTile + GhostTypes::TileDelta;
          vTileD = vTile;
          break;
        case DOWN:
          vTileD = vTile + GhostTypes::TileDelta;
          hTileD = hTile;
          break;
        }

        // If we hit a pipe and facing right, lock updates.
        if ((GhostTypes::PIPELINE ==
            GhostArena::Instance()->GetTileType(hTileD, vTileD)) &&
            (RIGHT == Direction))
        {
          LockUpdate();
          return;
        }

        if (0 == TransitionFrame)
        {
          ++TransitionFrame;

          // Adjust speed
          TransitionMax = GhostTypes::BaseTransition * 2;

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

// --------------------------------------------------------------------------------------
// Function: SetTiles
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::SetTiles(int32_t h, int32_t v)
{
  HTile = h;
  VTile = v;
}

// --------------------------------------------------------------------------------------
// Function: GetTiles
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::GetTiles(int32_t& h, int32_t& v)
{
  h = HTile;
  v = VTile;
}

// --------------------------------------------------------------------------------------
// Function: SetX
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::SetX(float x)
{
  X = x;
  Sprite->SetX(X - GhostTypes::BorderPadding);
}

// --------------------------------------------------------------------------------------
// Function: GetX
// Notes: None
// --------------------------------------------------------------------------------------
float FruitController::GetX() const
{
  return X;
}

// --------------------------------------------------------------------------------------
// Function: SetY
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::SetY(float y)
{
  Y = y;
  Sprite->SetY(Y - GhostTypes::BorderPadding);
}

// --------------------------------------------------------------------------------------
// Function: GetY
// Notes: None
// --------------------------------------------------------------------------------------
float FruitController::GetY() const
{
  return Y;
}

// --------------------------------------------------------------------------------------
// Function: SetRandomDirection
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::SetRandomDirection()
{
  int32_t hTile = HTile;
  int32_t vTile = VTile;
  int32_t hTileD = hTile;
  int32_t vTileD = vTile;

  // Add all legal directions
  std::vector<DirectionType> directions;

  if (true == PlayAnimation)
  {
    directions.push_back(Direction);
  }

  switch (Direction)
  {
  case LEFT:
    directions.push_back(UP);
    directions.push_back(DOWN);
    directions.push_back(RIGHT);
    break;
  case RIGHT:
    directions.push_back(UP);
    directions.push_back(DOWN);
    directions.push_back(LEFT);
    break;
  case UP:
    directions.push_back(LEFT);
    directions.push_back(RIGHT);
    directions.push_back(DOWN);
    break;
  case DOWN:
    directions.push_back(LEFT);
    directions.push_back(RIGHT);
    directions.push_back(UP);
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

    // Move in a legal direction as long as the destination is not a pipeline.
    DirectionType dir = directions[dirIndex];
    if (true == GhostArena::Instance()->IsMovementLegal(hTile, vTile, dir))
    {
      switch (dir)
      {
      case LEFT:
        --hTileD;
        break;
      case RIGHT:
        ++hTileD;
        break;
      case UP:
        --vTileD;
        break;
      case DOWN:
        ++vTileD;
        break;
      }
   
      if (GhostTypes::PIPELINE != GhostArena::Instance()->GetTileType(hTileD, vTileD))
      {
        DirRequest = dir;
        break; // Found new direction
      }
    }

    directions.erase(directions.begin() + dirIndex);
  }
}

// --------------------------------------------------------------------------------------
// Function: SetDirection
// Notes: None
// --------------------------------------------------------------------------------------
void FruitController::FindPathAStar(int32_t targetH,
                                    int32_t targetV,
                                    std::vector<DirectionType>& path)
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
    DirectionType pDir;

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
  int32_t currH = HTile;
  int32_t currV = VTile;
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

    for (uint32_t dir = RIGHT; dir <= DOWN; ++dir)
    {
      float newGCost = 0.0f;
      float newHCost = 0.0f;
      float newFCost = 0.0f;

      int32_t nextH = currH;
      int32_t nextV = currV;
      switch (dir)
      {
      case RIGHT:
        ++nextH;
        break;
      case LEFT:
        --nextH;
        break;
      case UP:
        --nextV;
        break;
      case DOWN:
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
          path.push_back(static_cast<DirectionType>(dir));

          // Add rest of path
          while (((HTile != searchMap[currIndex].pH) ||
                  (VTile != searchMap[currIndex].pV)) &&
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
            searchMap[nextIndex].pDir = static_cast<DirectionType>(dir);
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
// Function: FruitController
// Notes: None
// --------------------------------------------------------------------------------------
FruitController::FruitController() :
  SkipUpdate(true),
  X(0),
  Y(0),
  DestX(0),
  DestY(0),
  DeltaX(0),
  DeltaY(0),
  HTile(13),
  VTile(17),
  Sprite(nullptr),
  TransitionMax(8),
  TransitionFrame(0),
  FruitId(1),
  PlayAnimation(false),
  Direction(LEFT),
  DirRequest(NO_DIRECTION)
{

}

// --------------------------------------------------------------------------------------
// Function: ~FruitController
// Notes: None
// --------------------------------------------------------------------------------------
FruitController::~FruitController()
{

}