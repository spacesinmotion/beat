#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "game/GameTypes.h"
#include "game/SceneObject.h"
#include "math/Rect.h"
#include "math/Vec2.h"

typedef struct PointOverview PointOverview;
typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

typedef struct Stuff {
  int water, food, construction_material;
} Stuff;

typedef struct GameScene GameScene;
typedef void (*OnClickCB)(GameScene *, int id);
typedef struct PickRect {
  Rect rect;
  OnClickCB click;
  int id;
} PickRect;

typedef struct DiceRoll {
  ObjectType o;
  Vec2 p;
  float s;
  bool entered;
} DiceRoll;

typedef struct GameScene {
  SceneObjectVec scene_objects;

  Vec2 mouse_pos;

  PickRect pick_rects[32];
  int pick_rect_count;
  int pick_under_mouse;

  ObjectType grid[7][7];
  bool visited[7][7];

  PointOverview *points;

  DiceRoll dice[2];
  int selected_dice;
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void gs_add_object(GameScene *gs, SceneObject so);

#endif