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

typedef struct GameScene {
  SceneObjectVec scene_objects;

  float daytime;
  int day;

  Color preview;
  Recti r;

  int menu_under_mouse;
  int menu_selected;

  Level *level;
  StreetMap *street_map;

  Wearisome *w;
} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void GameScene_add_object(GameScene *gs, SceneObject so);

#endif