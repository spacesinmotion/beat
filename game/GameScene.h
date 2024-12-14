#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "SceneObject.h"

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

typedef struct GameScene {
  SceneObjectVec scene_objects;
  const sg_image *tilemap_img;
  const sg_image *menubar_img;
  const sg_image *marker;
  Vec2 mp;

  int menu_under_mouse;
  int menu_selected;

} GameScene;

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void GameScene_add_object(GameScene *gs, SceneObject so);

#endif