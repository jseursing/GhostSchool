#pragma once
#include "GhostTypes.h"

// Forward declarations
class OGLSprite;
class OGLText;
class OGLTimedSprite;

class CutScenes
{
public:

  static CutScenes* Instance();
  void StartCutScene(uint32_t act);
  void Render(uint32_t tick);
  bool IsFinished();

private:

  void RenderAct1(uint32_t tick);
  void RenderAct2(uint32_t tick);
  void RenderAct3(uint32_t tick);
  CutScenes();
  ~CutScenes();
  
  GhostTypes::ActTypeEnum CurrentAct;
  uint32_t ActStartTick;
  GhostTypes::DirectionTypeEnum MsPacmanDir;
  GhostTypes::DirectionTypeEnum PacmanDir;
  OGLSprite* MSPacman;
  OGLSprite* Pacman;
  OGLSprite* Pinky;
  OGLSprite* Inky;
  OGLSprite* Bird;
  OGLSprite* Junior;
  OGLSprite* Heart;
  OGLTimedSprite* ActSprite;
  OGLText* ActText;
};