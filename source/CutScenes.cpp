#include "CutScenes.h"
#include "GhostSchool.h"
#include "OGLExt.h"
#include "OGLSprite.h"
#include "OGLText.h"
#include "OGLTimedSprite.h"
#include "SoundEngine.h"

// --------------------------------------------------------------------------------------
// Function: Instance
// Notes: None
// --------------------------------------------------------------------------------------
CutScenes* CutScenes::Instance()
{
  static CutScenes instance;
  return &instance;
}

// --------------------------------------------------------------------------------------
// Function: StartCutScene
// Notes: None
// --------------------------------------------------------------------------------------
void CutScenes::StartCutScene(uint32_t act)
{
  // Set the current Act and start audio.
  ActStartTick = 0;
  CurrentAct = static_cast<GhostTypes::ActTypeEnum>(act);

  // Update ActText
  switch (CurrentAct)
  {
  case GhostTypes::ACT1:
    ActText->SetCurrentText("ACT 1  THEY MEET");
    break;
  case GhostTypes::ACT2:
    ActText->SetCurrentText("ACT 2  THE CHASE");
    break;
  case GhostTypes::ACT3:
    ActText->SetCurrentText("ACT 3  JUNIOR");
    break;
  }

  // Begin ACT render
  ActText->EnableRender();
  ActSprite->StartRender(50);
  ActSprite->SetActiveFrame(0);
}

// --------------------------------------------------------------------------------------
// Function: Render
// Notes: None
// --------------------------------------------------------------------------------------
void CutScenes::Render(uint32_t tick)
{
  glActiveTexture(GL_TEXTURE0);

  if (0 != ActSprite->GetRemainingTicks())
  {
    ActSprite->Render(0);
    ActText->Render(1);

    if ((25 == ActSprite->GetRemainingTicks()) ||
        (15 == ActSprite->GetRemainingTicks()))
    {
      ActSprite->SetActiveFrame(1);
    }
    else if (20 == ActSprite->GetRemainingTicks())
    {
      ActSprite->SetActiveFrame(2);
    }
    else if (10 == ActSprite->GetRemainingTicks())
    {
      ActSprite->SetActiveFrame(0);
      ActText->SetCurrentText(" ");
    }
  }
  else
  {
    if (0 == ActStartTick)
    {
      ActStartTick = tick;
    }

    switch (CurrentAct)
    {
    case GhostTypes::ACT1:
      RenderAct1(tick - ActStartTick);
      break;
    case GhostTypes::ACT2:
      RenderAct2(tick - ActStartTick);
      break;
    case GhostTypes::ACT3:
      RenderAct3(tick - ActStartTick);
      break;
    }
  }
}

// --------------------------------------------------------------------------------------
// Function: IsFinished
// Notes: None
// --------------------------------------------------------------------------------------
bool CutScenes::IsFinished()
{
  return (false == 
          SoundEngine::Instance()->IsActive(GhostTypes::AUDIO_ACT_ID[CurrentAct]));
}

// --------------------------------------------------------------------------------------
// Function: RenderAct1
// Notes: None
// --------------------------------------------------------------------------------------
void CutScenes::RenderAct1(uint32_t tick)
{
  static uint32_t frameTick = 0;
  static uint32_t animationTick = 0;

  if (0 == (tick % GhostTypes::AnimationFreq))
  {
    ++animationTick;
    if (0 == (animationTick % (GhostTypes::AnimationFreq * 2)))
    {
      ++frameTick;
    }

    // Begin start of act..
    if (0 == tick)
    {
      PacmanDir = GhostTypes::RIGHT;
      Pacman->SetX(-25);
      Pacman->SetY(350);
      Inky->SetX(-100);
      Inky->SetY(350);

      MsPacmanDir = GhostTypes::LEFT;
      MSPacman->SetX(GhostTypes::WindowWidth + 25);
      MSPacman->SetY(450);
      Pinky->SetX(GhostTypes::WindowWidth + 100);
      Pinky->SetY(450);
    }
    else
    {
      // First direction switch
      if ((GhostTypes::RIGHT == PacmanDir) &&
          (Inky->GetAbsX() > GhostTypes::WindowWidth + 25))
      {
        MsPacmanDir = GhostTypes::RIGHT;
        MSPacman->SetX(-25);
        MSPacman->SetY(400);
        Pinky->SetX(-100);
        Pinky->SetY(400);

        PacmanDir = GhostTypes::LEFT;
        Pacman->SetX(GhostTypes::WindowWidth + 25);
        Pacman->SetY(400);
        Inky->SetX(GhostTypes::WindowWidth + 100);
        Inky->SetY(400);
      }

      // Second direction switch
      if ((GhostTypes::LEFT == PacmanDir) &&
          (Pacman->GetAbsX() <= (MSPacman->GetAbsX() + 50)))
      {
        MsPacmanDir = GhostTypes::UP;
        PacmanDir = GhostTypes::UP;
      }

      // Last direction switch
      bool shouldUpdatePos = true;
      if ((GhostTypes::UP == PacmanDir) &&
          (Pacman->GetAbsY() <= 300))
      {
        shouldUpdatePos = false;
        Pacman->SetActiveFrame(3);
        MSPacman->SetActiveFrame(1);
        Heart->EnableRender();
      }

      if (true == shouldUpdatePos)
      {
        // Update sprites per direction
        if (GhostTypes::RIGHT == PacmanDir)
        {
          float currX = Pacman->GetAbsX() + 6;
          Pacman->SetX(currX);
          Pacman->SetActiveFrame(frameTick % 2);

          currX = Inky->GetAbsX() + 6;
          Inky->SetX(currX);
          Inky->SetActiveFrame(frameTick % 2);
        
          currX = MSPacman->GetAbsX() - 6;
          MSPacman->SetX(currX);
          MSPacman->SetActiveFrame(3 + (frameTick % 2));
        
          currX = Pinky->GetAbsX() - 6;
          Pinky->SetX(currX);
          Pinky->SetActiveFrame(2 + (frameTick % 2));
        }
        else if (GhostTypes::LEFT == PacmanDir)
        {
          float currX = Pacman->GetAbsX() - 6;
          Pacman->SetX(currX);
          Pacman->SetActiveFrame(2 + (frameTick % 2));

          currX = Inky->GetAbsX() - 6;
          Inky->SetX(currX);
          Inky->SetActiveFrame(2 + (frameTick % 2));

          currX = MSPacman->GetAbsX() + 6;
          MSPacman->SetX(currX);
          MSPacman->SetActiveFrame(frameTick % 2);

          currX = Pinky->GetAbsX() + 6;
          Pinky->SetX(currX);
          Pinky->SetActiveFrame(frameTick % 2);
        }
        else if (GhostTypes::UP == PacmanDir)
        {
          float currY = Pacman->GetAbsY() - 6;
          if (currY > 200)
          {
            Pacman->SetY(currY);
            Pacman->SetActiveFrame(4 + (frameTick % 2));

            MSPacman->SetY(currY);
            MSPacman->SetActiveFrame(6 + (frameTick % 2));

            if (50 < (Inky->GetAbsX() - Pinky->GetAbsX()))
            {
              float currX = Inky->GetAbsX() - 6;
              Inky->SetX(currX);
              Inky->SetActiveFrame(2 + (frameTick % 2));

              currX = Pinky->GetAbsX() + 6;
              Pinky->SetX(currX);
              Pinky->SetActiveFrame(frameTick % 2);
            }
          }
          else
          {
            Pacman->SetActiveFrame(0);
            MSPacman->SetActiveFrame(2);
            Heart->EnableRender();
          }
        }
      }
    }
  }

  Pacman->Render(0);
  MSPacman->Render(1);
  Pinky->Render(2);
  Inky->Render(3);
  Heart->Render(4);
}


// --------------------------------------------------------------------------------------
// Function: RenderAct2
// Notes: None
// --------------------------------------------------------------------------------------
void CutScenes::RenderAct2(uint32_t tick)
{
  static uint32_t frameTick = 0;
  static uint32_t animationTick = 0;
  static float xDelta = 6.0f;

  if (0 == (tick % GhostTypes::AnimationFreq))
  {
    ++animationTick;
    if (0 == (animationTick % (GhostTypes::AnimationFreq * 2)))
    {
      ++frameTick;
    }

    // Begin start of act.. MSPacman chasing Pacman
    if (0 == tick)
    {
      PacmanDir = GhostTypes::RIGHT;
      Pacman->SetX(-25);
      Pacman->SetY(201);

      MsPacmanDir = GhostTypes::RIGHT;
      MSPacman->SetX(-225);
      MSPacman->SetY(200);
    }
    else
    {
      if (6.0f == xDelta)
      {
        // First direction change.. Pacman chasing MSPacman
        if ((GhostTypes::RIGHT == PacmanDir) &&
            (MSPacman->GetAbsX() > GhostTypes::WindowWidth + 125) &&
            (MSPacman->GetAbsX() < Pacman->GetAbsX()))
        {
          MsPacmanDir = GhostTypes::LEFT;
          MSPacman->SetX(GhostTypes::WindowWidth + 50);
          MSPacman->SetY(400);

          PacmanDir = GhostTypes::LEFT;
          Pacman->SetX(GhostTypes::WindowWidth + 250);
          Pacman->SetY(401);

          if (800 < tick)
          {
            xDelta = 25.0f;
          }
        }

        // Second Direction Change.. Pacman chasing MSPacman
        if ((GhostTypes::LEFT == PacmanDir) &&
            (MSPacman->GetAbsX() < -225) &&
            (MSPacman->GetAbsX() > Pacman->GetAbsX()))
        {
          MsPacmanDir = GhostTypes::RIGHT;
          MSPacman->SetX(-250);
          MSPacman->SetY(400);

          PacmanDir = GhostTypes::RIGHT;
          Pacman->SetX(-50);
          Pacman->SetY(401);
        }
      }
      else
      {
        // Third direction change.. Pacman chasing MSPacman
        if ((GhostTypes::RIGHT == PacmanDir) &&
            (MSPacman->GetAbsX() > GhostTypes::WindowWidth + 150) &&
            (MSPacman->GetAbsX() < Pacman->GetAbsX()))
        {
          MsPacmanDir = GhostTypes::LEFT;
          MSPacman->SetX(GhostTypes::WindowWidth + 150);

          PacmanDir = GhostTypes::LEFT;
          Pacman->SetX(GhostTypes::WindowWidth + 350);

          if (700 < tick) // Finish..
          {
            MSPacman->DisableRender(); 
            Pacman->DisableRender();
            xDelta = 6.0f;
          }
        }

        // Fourth Direction Change.. Pacman chasing MSPacman
        if ((GhostTypes::LEFT == PacmanDir) &&
            (Pacman->GetAbsX() < -250) &&
            (Pacman->GetAbsX() > MSPacman->GetAbsX()))
        {
          MsPacmanDir = GhostTypes::RIGHT;
          MSPacman->SetX(-150);

          PacmanDir = GhostTypes::RIGHT;
          Pacman->SetX(-350);
        }
      }
    }

    if (GhostTypes::RIGHT == PacmanDir)
    {
      float currX = Pacman->GetAbsX() + xDelta;
      Pacman->SetX(currX);
      Pacman->SetActiveFrame(frameTick % 2);

      currX = MSPacman->GetAbsX() + xDelta + 1;
      MSPacman->SetX(currX);
      MSPacman->SetActiveFrame(frameTick % 2);
    }
    else if (GhostTypes::LEFT == PacmanDir)
    {
      float currX = Pacman->GetAbsX() - xDelta - 1;
      Pacman->SetX(currX);
      Pacman->SetActiveFrame(2 + (frameTick % 2));

      currX = MSPacman->GetAbsX() - xDelta;
      MSPacman->SetX(currX);
      MSPacman->SetActiveFrame(3 + (frameTick % 2));
    }
  }

  Pacman->Render(0);
  MSPacman->Render(1);
}

// --------------------------------------------------------------------------------------
// Function: RenderAct3
// Notes: None
// --------------------------------------------------------------------------------------
void CutScenes::RenderAct3(uint32_t tick)
{
  static uint32_t frameTick = 0;
  static uint32_t animationTick = 0;
  static float xDelta = 10.0f;
  static float yDelta = 0.0f;
  static float juniorXDelta = xDelta;

  if (0 == (tick % GhostTypes::AnimationFreq))
  {
    ++animationTick;
    if (0 == (animationTick % (GhostTypes::AnimationFreq * 2)))
    {
      ++frameTick;
    }

    Bird->SetActiveFrame(frameTick % 2);

    // Begin start of act.. MSPacman chasing Pacman
    if (0 == tick)
    {
      PacmanDir = GhostTypes::RIGHT;
      Pacman->SetActiveFrame(1);
      Pacman->SetX(150);
      Pacman->SetY(601);

      MsPacmanDir = GhostTypes::RIGHT;
      MSPacman->SetActiveFrame(1);
      MSPacman->SetX(200);
      MSPacman->SetY(600);

      Bird->SetX(GhostTypes::WindowWidth + 100);
      Bird->SetY(200);

      Junior->SetX(Bird->GetAbsX());
      Junior->SetY(Bird->GetAbsY() + 25);
      Junior->SetActiveFrame(0);
    }
    else
    {
      // Check for bird position to drop the baby (lol)
      if (400 > Junior->GetAbsX())
      {
        juniorXDelta = 1.0f;
        yDelta = 5.0f;
      }
    }

    if (-100.0f < Bird->GetAbsX())
    {
      float currX = Bird->GetAbsX() - xDelta;
      Bird->SetX(currX);
    }

    if (615 > Junior->GetAbsY())
    {
      float currX = Junior->GetAbsX() - juniorXDelta;
      Junior->SetX(currX);

      float currY = Junior->GetAbsY() + yDelta;
      Junior->SetY(currY);
    }
    else
    {
      // Finish
      xDelta = 10.0f;
      yDelta = 0.0f;
      juniorXDelta = xDelta;
      Junior->SetActiveFrame(1);
    }
  }

  Pacman->Render(0);
  MSPacman->Render(1);
  Bird->Render(2);
  Junior->Render(3);
}

// --------------------------------------------------------------------------------------
// Function: CutScenes
// Notes: None
// --------------------------------------------------------------------------------------
CutScenes::CutScenes() :
  CurrentAct(GhostTypes::ACT1),
  ActStartTick(0)
{
  // Construct ACT Sprite
  ActSprite = new OGLTimedSprite(GhostSchool::Instance()->GetProgramId(),
                                 GhostTypes::Sprite_Act,
                                 GhostTypes::WindowWidth,
                                 GhostTypes::WindowHeight,
                                 3,
                                 1);
  ActSprite->SetSize(75, 75);
  ActSprite->SetX(250);
  ActSprite->SetY(300);

  // Construct ACT Text
  ActText = new OGLText(GhostSchool::Instance()->GetProgramId(),
                        GhostTypes::Sprite_Font,
                        GhostTypes::WindowWidth,
                        GhostTypes::WindowHeight,
                        26,
                        5);
  ActText->RegisterCharSet('A', 'Z', 26);
  ActText->RegisterCharSet('0', '9', 0);
  ActText->SetSize(20, GhostTypes::PlayerSize);
  ActText->SetX(255);
  ActText->SetY(345);
  ActText->SetSpacing(12);

  // Construct MsPacman and Pacman
  MSPacman = new OGLSprite(GhostSchool::Instance()->GetProgramId(), 
                           GhostTypes::Sprite_Player,
                           GhostTypes::WindowWidth,
                           GhostTypes::WindowHeight,
                           3,
                           4);
  MSPacman->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);

  Pacman = new OGLSprite(GhostSchool::Instance()->GetProgramId(), 
                         GhostTypes::Sprite_Pacman,
                         GhostTypes::WindowWidth,
                         GhostTypes::WindowHeight,
                         2,
                         4);
  Pacman->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);

  // Construct Ghosts
  Pinky = new OGLSprite(GhostSchool::Instance()->GetProgramId(),
                        GhostTypes::Sprite_Ghost[1],
                        GhostTypes::WindowWidth,
                        GhostTypes::WindowHeight,
                        16,
                        1);
  Pinky->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);

  Inky = new OGLSprite(GhostSchool::Instance()->GetProgramId(),
                        GhostTypes::Sprite_Ghost[2],
                        GhostTypes::WindowWidth,
                        GhostTypes::WindowHeight,
                        16,
                        1);
  Inky->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);

  // Construct Bird and Junior
  Bird = new OGLSprite(GhostSchool::Instance()->GetProgramId(),
                        GhostTypes::Sprite_Bird,
                        GhostTypes::WindowWidth,
                        GhostTypes::WindowHeight,
                        2,
                        1);
  Bird->SetSize(GhostTypes::PlayerSize * 2, GhostTypes::PlayerSize);

  Junior = new OGLSprite(GhostSchool::Instance()->GetProgramId(),
                         GhostTypes::Sprite_Junior,
                         GhostTypes::WindowWidth,
                         GhostTypes::WindowHeight,
                         2,
                         1);
  Junior->SetSize(20, 20);

  // Construct Heart
  Heart = new OGLSprite(GhostSchool::Instance()->GetProgramId(),
                        GhostTypes::Sprite_Heart,
                        GhostTypes::WindowWidth,
                        GhostTypes::WindowHeight,
                        1,
                        1);
  Heart->SetSize(GhostTypes::PlayerSize, GhostTypes::PlayerSize);
  Heart->SetX(GhostTypes::WindowWidth / 2);
  Heart->SetY(265);
  Heart->DisableRender();
}

// --------------------------------------------------------------------------------------
// Function: ~CutScenes
// Notes: None
// --------------------------------------------------------------------------------------
CutScenes::~CutScenes()
{

}