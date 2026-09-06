#ifndef GAME_SCENE
#define GAME_SCENE

#include "engine/Game.h"
#include "engine/SceneObject.h"
#include "engine/math/Rect.h"
#include "game/TileContent.h"

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

typedef void (*OnClickCB)(GameScene *, int id);
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
  int day;
  bool a_new_day_just_started;

  int clicks, clicks_produced, clicks_lost, clicks_in_houses;
  int wearisome_count;
  Stuff resource_pool;
  Stuff resource_pool_claimed;
  Stuff resource_pool_spread;
  int storage_size, storage_claimed;

  Color preview;
  Recti r;

  int menu_selected;

  Level *level;
  StreetMap *street_map;

  G_Text click_counter_text;
  int click_counter_text_cache;
  G_Text water_counter_text;
  int water_counter_text_cache;
  G_Text food_counter_text;
  int food_counter_text_cache;
  G_Text construction_material_counter_text;
  int construction_material_counter_text_cache;
  G_Text free_storage_text;
  int free_storage_text_cache;
  G_Text day_counter_text;
  int day_counter_text_cache;
  G_Text bot_counter_text;
  int bot_counter_text_cache;

  Vec2 mouse_overlay_position;

  PickRect pick_rects[32];
  int pick_rect_count;
  int pick_under_mouse;

  ClickCBx special_click_handler;
  void *special_click_handler_data;

  float research_level, reasearch_needed;
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

static inline int gs_free_storage(GameScene *gs) {
  return gs->storage_size - gs->storage_claimed - gs->resource_pool.water - gs->resource_pool.food -
         gs->resource_pool.construction_material;
}

static inline int gs_resource(const GameScene *gs, Resource r) {
  if (r == R_Water)
    return gs->resource_pool.water + gs->resource_pool_spread.water;
  if (r == R_Food)
    return gs->resource_pool.food + gs->resource_pool_spread.food;
  if (r == R_ConstructionMaterial)
    return gs->resource_pool.construction_material + gs->resource_pool_spread.construction_material;
  return 0;
}

#endif