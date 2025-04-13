#ifndef GAMEGO_H
#define GAMEGO_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/SpaceShip.h"
#include "game/Word.h"
#include "game/WordBubble.h"
#include "math/Vec2.h"
#include <string.h>

typedef struct GameGo {
  Word *go;
} GameGo;

bool go_dead(const GameGo *go) { return go->go == NULL; }
float go_render_order(const GameGo *go) { return go->go->pos.y - 100; }

void go_update(GameGo *go, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)g;
  (void)dt;

  if (!go->go)
    return;
  if (go->go->char_reached == 2 && go->go->alpha > 1.0f) {
    w_destroy(go->go);
    go->go = NULL;
    WordBubble_init(gs, g);
    gs->initialized = 0.001f;
    return;
  }

  go->go->pos.y = gs->spaceship->pos.y - 2;
  go->go->pos = v_lerp(go->go->pos, v_add(gs->spaceship->pos, (Vec2){22.0f, 0.0f}), 0.01f);
}

void go_draw(GameGo *go, GameScene *gs, Game *g) {
  (void)gs;
  (void)g;
  (void)go;
}

SceneObjectTable GameGo_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)go_dead,
    .render_order = (SceneObjectRenderOrderCB)go_render_order,
    .update = (SceneObjectUpdateCB)go_update,
    .draw = (SceneObjectDrawCB)go_draw,
};

GameGo *GameGo_init(GameScene *gs, Game *g) {
  GameGo *go = (GameGo *)g_malloc(sizeof(GameGo));
  *go = (GameGo){.go = Word_init(gs, g, "go")};

  go->go->pos = v_add(gs->spaceship->pos, (Vec2){500, 0});

  gs_add_object(gs, (SceneObject){go, &GameGo_SceneObject_Table});

  return go;
}

#endif