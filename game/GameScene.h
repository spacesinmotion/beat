#ifndef GAME_SCENE
#define GAME_SCENE

#include "Game.h"
#include "SceneObject.h"

typedef struct GameScene GameScene;
void GameScene_init(Game *g);

void GameScene_draw(Game *g, GameScene *scene);
void GameScene_add_object(GameScene *gs, SceneObject so);

#endif