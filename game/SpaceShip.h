#ifndef SPACESHIP_H
#define SPACESHIP_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Vec2.h"

typedef struct SpaceShip {
  Vec2 pos;
} SpaceShip;

bool sh_dead(const SpaceShip *sh) { return sh->pos.y < 0.0; }
float sh_render_order(const SpaceShip *sh) { return sh->pos.y; }

void sh_update(SpaceShip *sh, GameScene *gs, Game *g, float dt) {
  (void)sh;
  (void)gs;
  (void)g;
  (void)dt;
}

void sh_draw(SpaceShip *sh, GameScene *gs, Game *g) {
  (void)gs;

  g_color(g, white());
  g_objectS(g, g_animation_buffer(g), Img_starship, 0, sh->pos, 2.0f);
}

SceneObjectTable SpaceShip_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)sh_dead,
    .render_order = (SceneObjectRenderOrderCB)sh_render_order,
    .update = (SceneObjectUpdateCB)sh_update,
    .draw = (SceneObjectDrawCB)sh_draw,
};

SpaceShip *SpaceShip_init(GameScene *gs, Vec2 pos) {
  SpaceShip *sh = (SpaceShip *)g_malloc(sizeof(SpaceShip));
  *sh = (SpaceShip){.pos = pos};

  gs_add_object(gs, (SceneObject){sh, &SpaceShip_SceneObject_Table});

  return sh;
}

#endif