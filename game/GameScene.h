#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "game/Player.h"
#include "game/SceneObject.h"
#include "math/Rect.h"

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

typedef void (*OnClickCB)(GameScene *, Game *, int);
typedef struct PickRect {
  Recti rect;
  OnClickCB click;
  int id;
} PickRect;

typedef struct SpaceShip SpaceShip;
typedef struct GameScene {
  SceneObjectVec scene_objects;

  float game_speed;
  bool game_paused;

  int menu_selected;

  Player user;

  Color preview;
  Recti r;

  Vec2 mouse_overlay_position;

  char entered_until_now[32];
  char entered_until_now_back[32];

  PickRect pick_rects[32];
  int pick_rect_count;
  int pick_under_mouse;

  SpaceShip *spaceship;

  int points, points_cache;
  G_Object points_text;
  float points_flush;

  float health, health_cache;
  G_Object health_text;
  float health_flush;

  float range, speed, energy;
  int level;
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void gs_add_object(GameScene *gs, SceneObject so);

static inline void gs_add_points(GameScene *gs, int p) {
  gs->points += p;
  gs->points_flush = 1.0f;
}

#endif