#pragma once
#include <stdint.h>


class GhostTypes
{
public:

  // THis header file constants constants used by Ghost School
  enum TileTypeEnum
  {
    WALL,
    SPELLET,
    BPELLET,
    PIPELINE,
    NOTHING
  };

  // Ghost constants
  enum GhostTypeEnum
  {
    BLINKY,  // Red
    PINKY,   // Pink
    INKY,    // BLUE
    SUE,    // Orange
    MAX_GHOST
  };

  enum ArenaEnum
  {
    ARENA0,
    ARENA1,
    ARENA2,
    ARENA3,
    ARENA4,
    ARENA5,
    MAX_ARENA
  };

  enum DirectionTypeEnum
  {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    VULNERABLE,
    EATEN,
    NO_DIRECTION
  };

  enum ActTypeEnum
  {
    ACT1,
    ACT2,
    ACT3,
    MAX_ACTS
  };

  // Size of the arena area (Gimp shows 225 x 248) - scale by 3
  static const float PlayerSize;
  static const float ArenaWidth;
  static const float ArenaHeight;
  static const float DisplayRatio;
  static const float WindowWidth;
  static const float WindowHeight; // Arena = Height - 100

  // Tile Mapping
  static const uint32_t TileDelta;
  static const uint32_t MaxFruits;
  static const uint32_t MaxLives;
  static const uint32_t HTileCount;
  static const uint32_t VTileCount;
  static const uint32_t TotalTiles;
  static const uint32_t BaseTransition;

  static uint32_t TileIndex(int32_t h, int32_t v);

  // How often to update (Less = Faster)
  static const uint32_t UpdateFreq;
  static const uint32_t AnimationFreq;
  static const uint32_t LoopCount;

  // Offset used for graphical purposes
  static const float BorderPadding;

  // OpenGL Shader paths
  static const char* VertexShader;
  static const char* FragmentShader;

  // Sprite resource paths
  static const char* Sprite_Hud;
  static const char* Sprite_Font;
  static const char* Sprite_Score;
  static const char* Sprite_Player;
  static const char* Sprite_Pacman;
  static const char* Sprite_Ghost[MAX_GHOST];
  static const char* Sprite_Arena;
  static const char* Sprite_Act;
  static const char* Sprite_Bird;
  static const char* Sprite_Junior;
  static const char* Sprite_Heart;
  static const char* Data_Arena[MAX_ARENA];

  // Audio resource paths and constants
  static uint32_t AUDIO_INTRO_ID;
  static uint32_t AUDIO_SPELLET_ID;
  static uint32_t AUDIO_BPELLET_ID;
  static uint32_t AUDIO_VULN_ID;
  static uint32_t AUDIO_RETREAT_ID;
  static uint32_t AUDIO_EAT_ID;
  static uint32_t AUDIO_DEATH_ID;
  static uint32_t AUDIO_EATFRUIT_ID;
  static uint32_t AUDIO_ADDLIFE_ID;
  static uint32_t AUDIO_GHOST_ID[MAX_GHOST];
  static uint32_t AUDIO_ACT_ID[MAX_ACTS];

  static const char* Audio_Intro;
  static const char* Audio_SPellet;
  static const char* Audio_BPellet;
  static const char* Audio_Vuln;
  static const char* Audio_Retreat;
  static const char* Audio_Eat;
  static const char* Audio_Death;
  static const char* Audio_EatFruit;
  static const char* Audio_AddLife;
  static const char* Audio_Ghost[MAX_GHOST];
  static const char* Audio_Act[MAX_ACTS];
};