#pragma once
#include "OGLApp.h"

// Forward definitions
class FruitController;
class GhostArena;
class GhostController;
class HudDisplay;
class OGLText;
class OGLTimedSprite;
class PacmanController;

// Class Definition
class GhostSchool : public OGLApp
{
public:

  static GhostSchool* Instance();
  void SetLevel(uint32_t level);
  void ResetGameplay();
  bool IsGameOver() const;
  void ProcessPlayerDeath();
  void DisplayScoreSprite(float x, float y, bool fruit, uint32_t frame);
  uint32_t GetGhostSpeedAdj() const;
  uint32_t GetPlayerSpeedAdj() const;
  void IncreasePoints(uint32_t val);
  void SetDelayedLockout(uint32_t ticks);
  void Run(int left, int top);

  float GetWidth() const;
  float GetHeight() const;
  GhostSchool(int argc, char** argv);
  ~GhostSchool();

  // -----------------
  // Debug flags
  // -----------------
  enum DebugFlag
  {
    GODMODE = 1
  };
  uint32_t DebugFlags;
  // -----------------

private:

  void CompileShaders();
  void ProcessNewGameState(uint32_t tick);
  void ProcessGameOverState(uint32_t tick);
  virtual void Initialize(OriginTypeEnum origin);
  virtual void DrawGraphics(uint32_t tick);
  virtual void OnKeyPressed(int32_t key);

  // Arena object
  bool Exit;
  bool NewGame;
  bool GameOver;
  uint32_t Points;
  uint32_t PlayerLives;
  uint32_t GhostSpeedAdj;
  uint32_t PlayerSpeedAdj;
  bool LivesUnlocked[2];

  GhostArena* Arena;
  OGLText* ScoreText;
  OGLText* StatusText;
  HudDisplay* GhostHud;
  FruitController* Fruit;
  GhostController* Ghosts;
  PacmanController* Player;
  OGLTimedSprite* ScoreSprite;
  uint32_t DelayedLockout;

  uint32_t ProgramId;
};