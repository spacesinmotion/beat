#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "SceneObject.h"
#include "math/Rect.h"

typedef struct Level Level;
typedef struct StreetMap StreetMap;
typedef struct Wearisome Wearisome;

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

typedef struct Stuff {
  int water, food, construction_material;
} Stuff;

typedef struct GameScene {
  SceneObjectVec scene_objects;

  float game_speed;
  bool game_paused;

  float daytime_step, daytime;
  int day;

  int clicks, clicks_produced, clicks_lost, clicks_in_houses;
  Stuff resource_pool;
  Stuff resource_pool_claimed;
  Stuff resource_pool_max;

  Color preview;
  Recti r;

  int menu_under_mouse;
  int menu_selected;

  Level *level;
  StreetMap *street_map;

  G_Object click_counter_text;
  int click_counter_text_cache;
  G_Object water_counter_text;
  int water_counter_text_cache;
  G_Object food_counter_text;
  int food_counter_text_cache;
  G_Object construction_material_counter_text;
  int construction_material_counter_text_cache;

  Vec2 mouse_overlay_position;
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void gs_add_object(GameScene *gs, SceneObject so);

static inline void gs_produce_click(GameScene *gs) {
  gs->clicks++;
  gs->clicks_produced++;
}
static inline void gs_loose_click(GameScene *gs) {
  gs->clicks--;
  gs->clicks_lost++;
}

#endif