#ifndef GAME_SCENE
#define GAME_SCENE

#include "engine/Scene.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "game/ObjectType.h"
#include "game/SceneObject.h"

typedef struct Board Board;
typedef struct PointOverview PointOverview;
typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

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

  Board *board;
  PointOverview *points;

  DiceRoll dice[2];
  int selected_dice, placed_dice;
  Sizei last_placed_dice_location;

  bool can_select_dice, no_move_left;

  float wobble_time;
} GameScene;

Scene GameScene_create();

void gs_add_object(GameScene *gs, SceneObject so);

#endif