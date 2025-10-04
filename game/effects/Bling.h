#ifndef BLING_H
#define BLING_H

#include "engine/Game.h"
#include "engine/math/Vec2.h"
#include "engine/math/random.h"
#include "engine/scene/SceneObject.h"
#include "game/assets.h"

typedef struct Bling {
  Color color;
  Vec2 location;
  float rotation;
  float time;
} Bling;

bool bl_die(Bling *bl, Game *g) {
  if (bl->time * 16.0f > 10.0f) {
    g_free(g, bl);
    return true;
  }
  return false;
}

void bl_update(Bling *bl, Game *g) { bl->time += g_animation_delta(g); }

float bl_render_order(Bling *bl) { return bl->location.y + 1000.0; }

void bl_draw(Bling *bl, Game *g) {

  const int frame = (int)(bl->time * 16.0f);
  g_color(g, bl->color);
  g_draw_icon(g, Img_bling, frame % 10, dt_pr(bl->location, bl->rotation));
}

static SceneObjectTable Bling_table = {
    .type = "Bling",
    .die = (SceneObjectDieCB)bl_die,
    .render_order = (SceneObjectRenderOrderCB)bl_render_order,
    .update = (SceneObjectUpdateCB)bl_update,
    .draw = (SceneObjectDrawCB)bl_draw,
};

Bling *Bling_create(Game *g, Vec2 l, Color c) {
  Bling *h = g_malloc(g, sizeof(Bling));
  *h = (Bling){
      .color = c,
      .location = l,
      .rotation = r_float_r(0, M_PI * 2.0f),
      .time = 0.0f,
  };
  return h;
}
#endif