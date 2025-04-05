#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "math/Rect.h"

typedef struct Level Level;
typedef struct StreetMap StreetMap;
typedef struct Wearisome Wearisome;

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

typedef struct Stuff {
  int money, food, wood, iron;
} Stuff;

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

  float daytime_step, daytime;
  int day, tick_of_day;

  Stuff resources;

  Color preview;
  Recti r;

  int menu_selected;

  Level *level;

  G_Object money_counter_text;
  int money_counter_text_cache;
  G_Object food_counter_text;
  int food_counter_text_cache;
  G_Object wood_counter_text;
  int wood_counter_text_cache;
  G_Object iron_counter_text;
  int iron_counter_text_cache;

  G_Object day_counter_text;
  int day_counter_text_cache;
  G_Object bot_counter_text;
  int bot_counter_text_cache;

  Vec2 mouse_overlay_position;

  PickRect pick_rects[32];
  int pick_rect_count;
  int pick_under_mouse;

  ClickCBx special_click_handler;
  void *special_click_handler_data;
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void gs_add_object(GameScene *gs, SceneObject so);

#endif