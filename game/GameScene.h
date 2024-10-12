#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "SceneObject.h"

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void GameScene_add_object(GameScene *gs, SceneObject so);

#endif