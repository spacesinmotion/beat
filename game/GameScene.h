#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "game/SceneObject.h"
#include "math/Rect.h"

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

typedef struct Stuff {
  int water, food, construction_material;
} Stuff;

typedef void (*OnClickCB)(GameScene *, int id);
typedef struct PickRect {
  Rect rect;
  OnClickCB click;
  int id;
} PickRect;

typedef enum ObjectType {
  So_Empty = 0,
  So_House,
  So_Trees,
  So_Animals,
  So_Flowers,
  So_Water,
  So_None,
} ObjectType;

typedef struct GroupCounter {
  G_Object text_3to5, text_6, text_g1, text_g2, text_points;
  int g1, g1_cache, g2, g2_cache, points, points_cache;
} GroupCounter;

typedef struct GameScene {
  SceneObjectVec scene_objects;

  int menu_selected;

  Vec2 mouse_pos;

  PickRect pick_rects[32];
  int pick_rect_count;
  int pick_under_mouse;

  ObjectType grid[7][7];
  bool visited[7][7];

  GroupCounter house;
  GroupCounter trees;
  GroupCounter animals;
  GroupCounter flowers;

  G_Object text_water_points;
  G_Object text_water_count;
  int water_count, water_count_cache;

  G_Object text_points;
  int points, points_cache;

  ObjectType dice[2];
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void gs_add_object(GameScene *gs, SceneObject so);

#endif