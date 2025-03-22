#ifndef DUST_H
#define DUST_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/assets.h"
#include "math/Vec2.h"

typedef struct Dust {
  Vec2 location;
  int start_frame, frame;
} Dust;

bool du_dead(Dust *du) { return du->frame > 16; }

void du_update(Dust *du, GameScene *gs, Game *g, float dt) {
  (void)g;
  (void)gs;
  (void)dt;

  du->frame += g_frame(g) - du->start_frame;
}

float du_render_order(Dust *du) { return du->location.y - 1000; }

void du_draw(Dust *du, GameScene *gs, Game *g) {
  (void)gs;

  g_color(g, gray(250));
  g_object(g, g_animation_buffer(g), Img_wearisome, du->frame / 6 + 4, du->location);
}

static SceneObjectTable Dust_table = {
    .type = "Dust",
    .dead = (SceneObjectDeadCB)du_dead,
    .render_order = (SceneObjectRenderOrderCB)du_render_order,
    .update = (SceneObjectUpdateCB)du_update,
    .draw = (SceneObjectDrawCB)du_draw,
};

Dust *Dust_init(Game *g, GameScene *gs, Vec2 l) {
  Dust *h = g_malloc(sizeof(Dust));
  *h = (Dust){
      .location = l,
      .frame = 0,
      .start_frame = g_frame(g),
  };

  gs_add_object(gs, (SceneObject){h, &Dust_table});

  return h;
}
#endif