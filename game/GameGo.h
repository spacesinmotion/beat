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

void go_word_entered(Word *w, GameScene *gs, Game *g, void *ud) {
  GameGo *go = (GameGo *)(ud);

  w_destroy(w);
  go->go = NULL;
  WordBubble_init(gs, g);
  gs->health = 100;
  gs->points = 0;
  gs->range = 0.0f;
  gs->speed = 1.0f;
  gs->energy = 5.0f;
  gs->level = 1;
  gs->initialized = 0.001f;
}

void go_update(GameGo *go, GameScene *gs, Game *g, float dt) {
  (void)g, (void)dt;

  if (go->go) {
    go->go->pos.y = gs->spaceship->pos.y - 2;
    go->go->pos = v_lerp(go->go->pos, v_add(gs->spaceship->pos, (Vec2){22.0f, 0.0f}), 0.01f);
  }
}

void go_draw(GameGo *go, GameScene *gs, Game *g) { (void)gs, (void)g, (void)go; }

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
  go->go->word_entered = go_word_entered;
  go->go->word_entered_user_data = go;

  gs_add_object(gs, (SceneObject){go, &GameGo_SceneObject_Table});

  return go;
}

#endif