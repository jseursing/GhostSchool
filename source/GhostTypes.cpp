#include "GhostTypes.h"

const float GhostTypes::PlayerSize   = 35.0f;
const float GhostTypes::ArenaWidth   = 224.0f;
const float GhostTypes::ArenaHeight  = 248.0f;
const float GhostTypes::DisplayRatio = 1.107f;
const float GhostTypes::WindowWidth  = 675.0f;
const float GhostTypes::WindowHeight = 844.0f; // Arena = Height - 100


const uint32_t GhostTypes::TileDelta      = 1;
const uint32_t GhostTypes::MaxFruits      = 7;
const uint32_t GhostTypes::MaxLives       = 3;
const uint32_t GhostTypes::HTileCount     = 28;
const uint32_t GhostTypes::VTileCount     = 31;
const uint32_t GhostTypes::TotalTiles     = HTileCount * VTileCount;
const uint32_t GhostTypes::BaseTransition = 8;

const uint32_t GhostTypes::UpdateFreq    = 2;
const uint32_t GhostTypes::AnimationFreq = 2;
const uint32_t GhostTypes::LoopCount     = 5;

const float GhostTypes::BorderPadding = 8.0f;

const char* GhostTypes::VertexShader   = ".\\resources\\glsl\\vertex.gls";
const char* GhostTypes::FragmentShader = ".\\resources\\glsl\\fragment.gls";

const char* GhostTypes::Sprite_Hud    = ".\\resources\\hud.png";
const char* GhostTypes::Sprite_Font   = ".\\resources\\font.png";
const char* GhostTypes::Sprite_Score  = ".\\resources\\scoresprite.png";
const char* GhostTypes::Sprite_Player = ".\\resources\\mpmsprite.png";
const char* GhostTypes::Sprite_Pacman = ".\\resources\\pmsprite.png";
const char* GhostTypes::Sprite_Ghost[MAX_GHOST] =
{
  ".\\resources\\ghost0.png",
  ".\\resources\\ghost1.png",
  ".\\resources\\ghost2.png",
  ".\\resources\\ghost3.png",
};
const char* GhostTypes::Sprite_Arena  = ".\\resources\\levelsprite.png";
const char* GhostTypes::Sprite_Act    = ".\\resources\\actsprite.png";
const char* GhostTypes::Sprite_Bird   = ".\\resources\\birdsprite.png";
const char* GhostTypes::Sprite_Junior = ".\\resources\\juniorsprite.png";
const char* GhostTypes::Sprite_Heart  = ".\\resources\\heartsprite.png";
const char* GhostTypes::Data_Arena[MAX_ARENA] =
{
  ".\\resources\\level0.dat",
  ".\\resources\\level1.dat",
  ".\\resources\\level2.dat",
  ".\\resources\\level3.dat",
  ".\\resources\\level4.dat",
  ".\\resources\\level5.dat"
};

uint32_t GhostTypes::AUDIO_INTRO_ID            = 0;
uint32_t GhostTypes::AUDIO_SPELLET_ID          = 0;
uint32_t GhostTypes::AUDIO_BPELLET_ID          = 0;
uint32_t GhostTypes::AUDIO_VULN_ID             = 0;
uint32_t GhostTypes::AUDIO_RETREAT_ID          = 0;
uint32_t GhostTypes::AUDIO_EAT_ID              = 0;
uint32_t GhostTypes::AUDIO_DEATH_ID            = 0;
uint32_t GhostTypes::AUDIO_EATFRUIT_ID         = 0;
uint32_t GhostTypes::AUDIO_ADDLIFE_ID          = 0;
uint32_t GhostTypes::AUDIO_GHOST_ID[MAX_GHOST] = { 0, 0, 0, 0 };
uint32_t GhostTypes::AUDIO_ACT_ID[MAX_ACTS] = {0, 0, 0};

const char* GhostTypes::Audio_Intro    = ".\\resources\\audio\\Start.wav";
const char* GhostTypes::Audio_SPellet  = ".\\resources\\audio\\SPellet.wav";
const char* GhostTypes::Audio_BPellet  = ".\\resources\\audio\\BPellet.wav";
const char* GhostTypes::Audio_Vuln     = ".\\resources\\audio\\Vulnerable.wav";
const char* GhostTypes::Audio_Retreat  = ".\\resources\\audio\\Retreat.wav";
const char* GhostTypes::Audio_Eat      = ".\\resources\\audio\\EatGhost.wav";
const char* GhostTypes::Audio_Death    = ".\\resources\\audio\\Death.wav";
const char* GhostTypes::Audio_EatFruit = ".\\resources\\audio\\EatFruit.wav";
const char* GhostTypes::Audio_AddLife  = ".\\resources\\audio\\ExtraLife.wav";
const char* GhostTypes::Audio_Ghost[MAX_GHOST] =
{
  ".\\resources\\audio\\Ghost0.wav",
  ".\\resources\\audio\\Ghost1.wav",
  ".\\resources\\audio\\Ghost2.wav",
  ".\\resources\\audio\\Ghost3.wav"
};
const char* GhostTypes::Audio_Act[MAX_ACTS] =
{
  ".\\resources\\audio\\Act1.wav",
  ".\\resources\\audio\\Act2.wav",
  ".\\resources\\audio\\Act3.wav"
};

uint32_t GhostTypes::TileIndex(int32_t h, int32_t v)
{
  return (v * HTileCount) + h;
}