#ifndef BLING_H
#define BLING_H

#include "SokEngWrap/Game.h"
#include "SokEngWrap/math/Vec2.h"
#include "SokEngWrap/math/random.h"
#include "game/GameScene.h"
#include "game/assets/textures.h"
#include "math.h"

typedef struct Bling {
  Color color;
  Vec2 location;
  float rotation;
  float time;
} Bling;

bool bl_dead(Bling *bl) { return bl->time * 16.0f > 10.0f; }

void bl_update(Bling *bl, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)dt;

  bl->time += g_animation_delta(g);
}

float bl_render_order(Bling *bl) { return bl->location.y + 1000.0; }

void bl_draw(Bling *bl, GameScene *gs, Game *g) {
  (void)gs;

  const int frame = (int)(bl->time * 16.0f);
  g_color(g, bl->color);
  g_objectR(g, g_animation_buffer(g), Img_bling, frame % 10, bl->location, bl->rotation);
}

static SceneObjectTable Bling_table = {
    .type = "Bling",
    .dead = (SceneObjectDeadCB)bl_dead,
    .render_order = (SceneObjectRenderOrderCB)bl_render_order,
    .update = (SceneObjectUpdateCB)bl_update,
    .draw = (SceneObjectDrawCB)bl_draw,
};

Bling *Bling_init(GameScene *gs, Vec2 l, Color c) {
  Bling *h = g_malloc(sizeof(Bling));
  *h = (Bling){
      .color = c,
      .location = l,
      .rotation = r_float_r(0, M_PI * 2.0f),
      .time = 0.0f,
  };

  gs_add_object(gs, (SceneObject){h, &Bling_table});

  return h;
}
#endif