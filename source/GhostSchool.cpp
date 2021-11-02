#include "FruitController.h"
#include "GhostArena.h"
#include "GhostController.h"
#include "GhostSchool.h"
#include "GhostTypes.h"
#include "HudDisplay.h"
#include "OGLExt.h"
#include "OGLSpriteMgr.h"
#include "OGLSprite.h"
#include "OGLText.h"
#include "OGLTimedSprite.h"
#include "PacmanController.h"
#include "SoundEngine.h"

// External dependencies
#include <fstream>
#include <math.h>

// Normally the project would be configured to link the following libraries,
// but they are placed here to avoid confusion.
#pragma comment(lib, "OGL.lib") // Internal

// Graphics
#pragma comment(lib, "GLu32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "SOIL.lib")

// Audio
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

// --------------------------------------------------------------------------------------
// Function: Instance
// Notes: None
// --------------------------------------------------------------------------------------
GhostSchool* GhostSchool::Instance()
{
  return static_cast<GhostSchool*>(MyInstance);
}

// --------------------------------------------------------------------------------------
// Function: SetLevel
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::SetLevel(uint32_t level)
{
  GhostHud->SetFruitLevel(level);
  Fruit->SetFruit(level);

  // Adjust speed
  switch (level)
  {
    // First level is easy, ghosts are slower
  case 0:
    PlayerSpeedAdj = -1;
    GhostSpeedAdj = -2;
    break;
    
    // Switch to normal speed.
  case 1:
  case 2:
  case 3:
  case 4:
    PlayerSpeedAdj = 0;
    GhostSpeedAdj = 0;
    break;
  
  // Red Arena adjust speed. Player and ghost speeds up by 1 tick.
  case 5:
  case 6:
  case 7:
  case 8:
    PlayerSpeedAdj = 1;
    GhostSpeedAdj = 1;
    break;

  // Baby arena and next has ghosts speed adjusted faster than player.
  case 9: 
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
    PlayerSpeedAdj = 1;
    GhostSpeedAdj = 2;
    break;

  // End game, both are faster..
  default:
    PlayerSpeedAdj = 2;
    GhostSpeedAdj = 4;
  }
}

// --------------------------------------------------------------------------------------
// Function: ResetGameplay
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::ResetGameplay()
{
  Player->ResetStartPosition();
  Player->ResetDestination();

  Ghosts->ResetStartPosition(GhostTypes::BLINKY);
  Ghosts->ResetStartPosition(GhostTypes::PINKY);
  Ghosts->ResetStartPosition(GhostTypes::INKY);
  Ghosts->ResetStartPosition(GhostTypes::SUE);
  Ghosts->ResetDestination(GhostTypes::BLINKY);
  Ghosts->ResetDestination(GhostTypes::PINKY);
  Ghosts->ResetDestination(GhostTypes::INKY);
  Ghosts->ResetDestination(GhostTypes::SUE);

  Arena->LockUpdate();
  Player->LockUpdate();
  Ghosts->LockUpdate();
  Fruit->LockUpdate();

  Points = 0;
  LivesUnlocked[0] = false;
  LivesUnlocked[1] = false;
  PlayerLives = GhostTypes::MaxLives;
  GhostHud->SetLives(PlayerLives);
  GhostHud->SetFruitLevel(0);
  Arena->SetLevel(0);
  Fruit->SetFruit(0);
  GameOver = false;
  NewGame = true;

  ScoreText->SetCurrentText("Score: 0");
  GameOverText->DisableRender();
  ScoreSprite->DisableRender();

  // Play intro song
  SoundEngine::Instance()->Play(GhostTypes::AUDIO_INTRO_ID);
}

// --------------------------------------------------------------------------------------
// Function: IsGameOver
// Notes: None
// --------------------------------------------------------------------------------------
bool GhostSchool::IsGameOver() const
{
  return GameOver;
}

// --------------------------------------------------------------------------------------
// Function: ProcessPlayerDeath
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::ProcessPlayerDeath()
{
  --PlayerLives;
  if (0 == PlayerLives)
  {
    GameOver = true;
  }
  else
  {
    GhostHud->SetLives(PlayerLives);
  }
}

// --------------------------------------------------------------------------------------
// Function: DisplayScoreSprite
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::DisplayScoreSprite(float x, float y, bool fruit, uint32_t frame)
{
  ScoreSprite->SetX(x - GhostTypes::BorderPadding);
  ScoreSprite->SetY(y - GhostTypes::BorderPadding);

  uint32_t activeFrame = (true == fruit ? 4 : 0) + frame;
  ScoreSprite->SetActiveFrame(activeFrame);
  ScoreSprite->StartRender(100);
}

// --------------------------------------------------------------------------------------
// Function: GetGhostSpeedAdj
// Notes: None
// --------------------------------------------------------------------------------------
uint32_t GhostSchool::GetGhostSpeedAdj() const
{
  return GhostSpeedAdj;
}

// --------------------------------------------------------------------------------------
// Function: GetPlayerSpeedAdj
// Notes: None
// --------------------------------------------------------------------------------------
uint32_t GhostSchool::GetPlayerSpeedAdj() const
{
  return PlayerSpeedAdj;
}

// --------------------------------------------------------------------------------------
// Function: IncreasePoints
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::IncreasePoints(uint32_t val)
{
  Points += val;

  std::string strScore = "Score: " + std::to_string(Points);
  ScoreText->SetCurrentText(strScore.c_str());

  // Handle extra lives
  if (false == LivesUnlocked[0])
  {
    if (Points >= 10000)
    {
      LivesUnlocked[0] = true;

      // Play extra life sound.
      SoundEngine::Instance()->Play(GhostTypes::AUDIO_ADDLIFE_ID);
      
      // Increase life
      ++PlayerLives;
      GhostHud->SetLives(PlayerLives);
    }
  }
  if (false == LivesUnlocked[1])
  {
    if (Points >= 20000)
    {
      LivesUnlocked[1] = true;

      // Play extra life sound.
      SoundEngine::Instance()->Play(GhostTypes::AUDIO_ADDLIFE_ID);

      // Increase life
      ++PlayerLives;
      GhostHud->SetLives(PlayerLives);
    }
  }
}

// --------------------------------------------------------------------------------------
// Function: SetDelayedLockout
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::SetDelayedLockout(uint32_t ticks)
{
  DelayedLockout = ticks;
}

// --------------------------------------------------------------------------------------
// Function: Run
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::Run(int left, int top)
{
  SetRefreshRate(60); // 60-FPS
  SetDimensions(left,
                static_cast<int32_t>(left + GhostTypes::WindowWidth),
                top,
                static_cast<int32_t>(top + GhostTypes::WindowHeight));
  SetWindowTitle("Ghost School");
  Initialize(TOP_LEFT);

  MainLoop();
}

// --------------------------------------------------------------------------------------
// Function: GhostSchool
// Notes: None
// --------------------------------------------------------------------------------------
float GhostSchool::GetWidth() const
{
  RECT dimensions;
  MyInstance->GetDimensions(dimensions);
  return (dimensions.right - dimensions.left) * 1.0f;
}

// --------------------------------------------------------------------------------------
// Function: GhostSchool
// Notes: None
// --------------------------------------------------------------------------------------
float GhostSchool::GetHeight() const
{
  RECT dimensions;
  MyInstance->GetDimensions(dimensions);
  return (dimensions.bottom - dimensions.top) * 1.0f;
}

// --------------------------------------------------------------------------------------
// Function: GhostSchool
// Notes: None
// --------------------------------------------------------------------------------------
GhostSchool::GhostSchool(int argc, char* argv[]) :
  OGLApp(argc, argv),
  Exit(false),
  NewGame(true),
  GameOver(false),
  PlayerLives(GhostTypes::MaxLives),
  GhostSpeedAdj(-2),
  PlayerSpeedAdj(-1),
  LivesUnlocked{false, false},
  Player(nullptr),
  ScoreSprite(nullptr),
  Arena(nullptr),
  ScoreText(nullptr),
  GameOverText(nullptr),
  DelayedLockout(0),
  DebugFlags(GODMODE) // TODO
{
}

// --------------------------------------------------------------------------------------
// Function: ~GhostSchool
// Notes: None
// --------------------------------------------------------------------------------------
GhostSchool::~GhostSchool()
{
  SoundEngine::Destroy();
}

// --------------------------------------------------------------------------------------
// Function: CompileShaders
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::CompileShaders()
{
  // Build GL Shaders
  char vertexBuffer[512] = { 0 };
  std::ifstream vertexStream(GhostTypes::VertexShader);
  if (false == vertexStream.is_open())
  {
    MessageBoxA(0, "Unable to locate vertex.gls", "Error", MB_OK);
    return;
  }
  vertexStream.read(vertexBuffer, sizeof(vertexBuffer));
  vertexStream.close();

  char fragmentBuffer[512] = { 0 };
  std::ifstream fragmentStream(GhostTypes::FragmentShader);
  if (false == fragmentStream.is_open())
  {
    MessageBoxA(0, "Unable to locate fragment.gls", "Error", MB_OK);
    return;
  }
  fragmentStream.read(fragmentBuffer, sizeof(fragmentBuffer));
  fragmentStream.close();

  ProgramId = OGLSpriteMgr::Instance()->BuildShaderGL(vertexBuffer,
                                                      fragmentBuffer);
}

// --------------------------------------------------------------------------------------
// Function: ProcessNewGameState
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::ProcessNewGameState(uint32_t tick)
{
  // If this is a new game, and tick == animation frequency, lock out updates.
  // Begin playing the intro song, and upon finish, unlock updates.
  if (0 == tick)
  {
    ResetGameplay();
    Arena->LockUpdate();
    Player->LockUpdate();
    Ghosts->LockUpdate();
    return;
  }
  
  if (false == SoundEngine::Instance()->IsActive(GhostTypes::AUDIO_INTRO_ID))
  {
    NewGame = false;
    Arena->UnlockUpdate();
    Player->UnlockUpdate();
    Ghosts->UnlockUpdate();
    ResetDisplayTick();
  }
}

// --------------------------------------------------------------------------------------
// Function: ProcessGameOverState
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::ProcessGameOverState(uint32_t tick)
{
  // Flash the GameOver text every 5 ticks
  uint32_t remTick = tick % 20;
  if (15 > remTick)
  {
    GameOverText->EnableRender();
  }
  else
  {
    GameOverText->DisableRender();
  }
}

// --------------------------------------------------------------------------------------
// Function: Initialize
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::Initialize(OriginTypeEnum origin)
{
  // Invoke parent init
  OGLApp::Initialize(origin);

  // Initialize Render Manager and sprites
  OGLSpriteMgr::Instance()->Initialize(GhostTypes::WindowWidth, 
                                       GhostTypes::WindowHeight);

  // Compile graphics shaders
  CompileShaders();

  // Initialize game objects in order of display
  Arena = GhostArena::Instance(ProgramId);
  Fruit = FruitController::Instance(ProgramId);

  ScoreSprite = new OGLTimedSprite(ProgramId,
                                   GhostTypes::Sprite_Score,
                                   GhostTypes::WindowWidth,
                                   GhostTypes::WindowHeight,
                                   11,
                                   1);
  ScoreSprite->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);
  OGLSpriteMgr::Instance()->RegisterSprite(ScoreSprite, "scorepopup");

  Player = PacmanController::Instance(ProgramId); 
  Ghosts = GhostController::Instance(ProgramId);

  ScoreText = new OGLText(ProgramId,
                          GhostTypes::Sprite_Font,
                          GhostTypes::WindowWidth,
                          GhostTypes::WindowHeight,
                          26,
                          5);
  OGLSpriteMgr::Instance()->RegisterSprite(ScoreText, "font");
  ScoreText->RegisterCharSet('0', '9', 0);
  ScoreText->RegisterCharSet('A', 'Z', 26);
  ScoreText->RegisterCharSet('a', 'z', 52);
  ScoreText->SetCharacterMapping(':', 120);
  ScoreText->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);
  ScoreText->SetX(10);
  ScoreText->SetY(GhostTypes::WindowHeight - 40);
  ScoreText->SetSpacing(20);
  ScoreText->SetCurrentText("Score: 0");

  GameOverText = new OGLText(ProgramId,
                             GhostTypes::Sprite_Font,
                             GhostTypes::WindowWidth,
                             GhostTypes::WindowHeight,
                             26,
                             5);
  OGLSpriteMgr::Instance()->RegisterSprite(GameOverText, "font");
  GameOverText->DisableRender();
  GameOverText->RegisterCharSet('A', 'Z', 26);
  GameOverText->RegisterCharSet('a', 'z', 52);
  GameOverText->SetCharacterMapping('-', 22);
  GameOverText->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);
  GameOverText->SetX(10);
  GameOverText->SetY(GhostTypes::WindowHeight - 75);
  GameOverText->SetSpacing(25);
  GameOverText->SetCurrentText("Game Over! Press Any Key");

  GhostHud = new HudDisplay(ProgramId,
                            GhostTypes::Sprite_Hud,
                            GhostTypes::WindowWidth,
                            GhostTypes::WindowHeight,
                            8,
                            1);
  OGLSpriteMgr::Instance()->RegisterSprite(GhostHud, "hud");
  GhostHud->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);
  GhostHud->SetX(10);
  GhostHud->SetY(GhostTypes::WindowHeight - 90);

  // Our window should now exist, initialize sound engine and effects
  if (true == SoundEngine::Initialize())
  {
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Intro, 
                                           GhostTypes::AUDIO_INTRO_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_SPellet, 
                                           GhostTypes::AUDIO_SPELLET_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_BPellet, 
                                           GhostTypes::AUDIO_BPELLET_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Vuln, 
                                           GhostTypes::AUDIO_VULN_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Retreat, 
                                           GhostTypes::AUDIO_RETREAT_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Eat, 
                                           GhostTypes::AUDIO_EAT_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Death, 
                                           GhostTypes::AUDIO_DEATH_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_EatFruit, 
                                           GhostTypes::AUDIO_EATFRUIT_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_AddLife, 
                                           GhostTypes::AUDIO_ADDLIFE_ID);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Ghost[0], 
                                           GhostTypes::AUDIO_GHOST_ID[0]);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Ghost[1], 
                                           GhostTypes::AUDIO_GHOST_ID[1]);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Ghost[2], 
                                           GhostTypes::AUDIO_GHOST_ID[2]);
    SoundEngine::Instance()->RegisterSound(GhostTypes::Audio_Ghost[3], 
                                           GhostTypes::AUDIO_GHOST_ID[3]);
  }
}

// --------------------------------------------------------------------------------------
// Function: DrawGraphics
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::DrawGraphics(uint32_t tick)
{
  // GL Draw Begin
  glClear(GL_COLOR_BUFFER_BIT);

  if (true == NewGame)
  {
    ProcessNewGameState(tick);
  }

  if (true == GameOver)
  {
    ProcessGameOverState(tick);
  }

  // Update arena, player, and draw renders
  if (0 == DelayedLockout)
  {
    Arena->Update(tick);
    Player->Update(tick);
    Ghosts->Update(tick);
    Fruit->Update(tick);
  }
  else
  {
    --DelayedLockout;
  }

  // Render all sprites
  OGLSpriteMgr::Instance()->Render();

  // Refresh
  OGLExt::GLSwapBuffers();
}

// --------------------------------------------------------------------------------------
// Function: OnKeyPressed
// Notes: None
// --------------------------------------------------------------------------------------
void GhostSchool::OnKeyPressed(int32_t key)
{
  if (true == GameOver)
  {
    GameOver = false;
    ResetGameplay();
  }
  else
  {
    switch (key)
    {
    case VK_LEFT:
      Player->ProcessKey(GhostTypes::LEFT);
      break;
    case VK_UP:
      Player->ProcessKey(GhostTypes::UP);
      break;
    case VK_RIGHT:
      Player->ProcessKey(GhostTypes::RIGHT);
      break;
    case VK_DOWN:
      Player->ProcessKey(GhostTypes::DOWN);
      break;
    }
  }
}