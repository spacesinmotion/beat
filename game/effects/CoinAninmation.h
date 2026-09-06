#ifndef CoinAnimation_H
#define CoinAnimation_H

#include "engine/Game.h"
#include "engine/math/Color.h"
#include "engine/math/Vec2.h"
#include "game/GameScene.h"
#include "game/assets.h"
#include "math.h"

typedef struct CoinAnimation {
  Vec2 location;
  float time;
} CoinAnimation;

bool ca_dead(CoinAnimation *ca) { return ca->time > 1.0f; }

void ca_update(CoinAnimation *ca, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)dt;

  ca->time += g_animation_delta(g);
}

float ca_render_order(CoinAnimation *ca) { return ca->location.y + 1000.0; }

void ca_draw(CoinAnimation *ca, GameScene *gs, Game *g) {
  (void)gs;

  Vec2 l = v_add(ca->location, (Vec2){0.0, ca->time * 16});
  g_color(g, alphaf(white(), 1.0f - ca->time * ca->time));
  g_objectRS(g, g_animation_buffer(g), Img_coin, 0, l, sin(ca->time * 16.0f) * 32.0f / 180.0f * M_PI,
             sin(ca->time * ca->time * M_PI));
}

static SceneObjectTable CoinAnimation_table = {
    .type = "CoinAnimation",
    .dead = (SceneObjectDeadCB)ca_dead,
    .render_order = (SceneObjectRenderOrderCB)ca_render_order,
    .update = (SceneObjectUpdateCB)ca_update,
    .draw = (SceneObjectDrawCB)ca_draw,
};

CoinAnimation *CoinAnimation_init(GameScene *gs, Vec2 l) {
  CoinAnimation *ca = g_malloc(sizeof(CoinAnimation));
  *ca = (CoinAnimation){
      .location = l,
      .time = 0.0f,
  };

  gs_add_object(gs, (SceneObject){ca, &CoinAnimation_table});

  return ca;
}
#endif