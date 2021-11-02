#pragma once
#include "OGLSprite.h"
#include <map>
#include <string>

class HudDisplay : public OGLSprite
{
public:

  void SetLives(uint32_t lives);
  void SetFruitLevel(uint32_t level);
  
  virtual void Render(uint32_t texId);
  virtual void SetX(float x);
  virtual void SetY(float y);

  HudDisplay(uint32_t programId,
             const char* imagePath,
             float windowW,
             float windowH,
             uint32_t hFrames,
             uint32_t vFrames);
  ~HudDisplay();

private:

  float LivesX;
  uint32_t LifeCount;
  
  float FruitX;
  uint32_t FruitLevel;

  float PosY;
};