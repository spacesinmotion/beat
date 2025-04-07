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

typedef struct GameScene {
  SceneObjectVec scene_objects;

  float game_speed;
  bool game_paused;

  int menu_selected;

  Player user;

  Color preview;
  Recti r;

  Vec2 mouse_overlay_position;

  G_Object word_start;
  G_Object word_end;
  int char_reached;
  float word_end_offset;

  char entered_until_now[32];
  char entered_until_now_back[32];
  G_Object entered;

  PickRect pick_rects[32];
  int pick_rect_count;
  int pick_under_mouse;
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void gs_add_object(GameScene *gs, SceneObject so);

#endif