#include "GhostTypes.h"
#include "HudDisplay.h"



// --------------------------------------------------------------------------------------
// Function: SetCharacterMapping
// Notes: None
// --------------------------------------------------------------------------------------
void HudDisplay::SetLives(uint32_t lives)
{
  LifeCount = lives - 1; // Display all lives except for active one
}

// --------------------------------------------------------------------------------------
// Function: SetFruitLevel
// Notes: None
// --------------------------------------------------------------------------------------
void HudDisplay::SetFruitLevel(uint32_t level)
{
  if (level < GhostTypes::MaxFruits)
  {
    FruitLevel = level;
  }
  else
  {
    FruitLevel = GhostTypes::MaxFruits - 1;
  }
}

// --------------------------------------------------------------------------------------
// Function: Render
// Notes: None
// --------------------------------------------------------------------------------------
void HudDisplay::Render(uint32_t texId)
{
  // The way we are rendering the hud is to render each object once for each
  // life/fruit specified.

  // Draw Lives
  SetActiveFrame(0);
  for (size_t i = 0; i < LifeCount; ++i)
  {
    OGLSprite::SetX(LivesX + (i * (GetPixelWidth() + 5)));
    OGLSprite::Render(texId);
  }

  // Draw fruits
  for (size_t i = 0; i <= FruitLevel; ++i)
  {
    SetActiveFrame(i + 1);
    OGLSprite::SetX(FruitX + (i * (GetPixelWidth() + 5)));
    OGLSprite::Render(texId);
  }
}

// --------------------------------------------------------------------------------------
// Function: SetX
// Notes: None
// --------------------------------------------------------------------------------------
void HudDisplay::SetX(float x)
{
  LivesX = x;
  OGLSprite::SetX(LivesX);

  FruitX = LivesX + (6 * (GetPixelWidth() + 5));
}

// --------------------------------------------------------------------------------------
// Function: SetY
// Notes: None
// --------------------------------------------------------------------------------------
void HudDisplay::SetY(float y)
{
  PosY = y;
  OGLSprite::SetY(PosY);
}

// --------------------------------------------------------------------------------------
// Function: HudDisplay
// Notes: None
// --------------------------------------------------------------------------------------
HudDisplay::HudDisplay(uint32_t programId,
                       const char* imagePath,
                       float windowW,
                       float windowH,
                       uint32_t hFrames,
                       uint32_t vFrames) :
  OGLSprite(programId, imagePath, windowW, windowH, hFrames, vFrames),
  LivesX(0),
  LifeCount(2),
  FruitX(0),
  FruitLevel(0),
  PosY(0)
{

}


// --------------------------------------------------------------------------------------
// Function: ~HudDisplay
// Notes: None
// --------------------------------------------------------------------------------------
HudDisplay::~HudDisplay()
{

}