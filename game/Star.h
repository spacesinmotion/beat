#ifndef STAR_H
#define STAR_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Vec2.h"
#include "math/random.h"

typedef struct Star {
  Vec2 pos, vel;
  float scale, rot, rot_vel, alpha_t, alpha_vel;
} Star;

bool st_dead(const Star *st) { return st->pos.y < 0.0; }
float st_render_order(const Star *st) { return st->pos.y - 100; }

void st_update(Star *st, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)g;
  (void)dt;

  st->pos = v_add(st->pos, v_mulf(st->vel, gs->speed * dt));
  st->rot += st->rot_vel * dt;
  st->alpha_t += dt * st->alpha_vel;

  if (st->pos.x < 0) {
    st->pos = (Vec2){400 + rand() % 200, 50 + rand() % 300};
  }
}

void st_draw(Star *st, GameScene *gs, Game *g) {
  (void)gs;

  g_color(g, alphaf(white(), 0.7f + 0.1f * sin(st->alpha_t)));
  g_objectRS(g, g_animation_buffer(g), Img_starship, 1, st->pos, st->rot, vec2f(st->scale));
}

SceneObjectTable Star_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)st_dead,
    .render_order = (SceneObjectRenderOrderCB)st_render_order,
    .update = (SceneObjectUpdateCB)st_update,
    .draw = (SceneObjectDrawCB)st_draw,
};

Star *Star_init(GameScene *gs) {
  Star *st = (Star *)g_malloc(sizeof(Star));
  *st = (Star){
      .pos = (Vec2){500 + rand() % 200, 50 + rand() % 300},
      .vel = (Vec2){-(10 + rand() % 30), 0},
      .scale = r_float_r(0.1f, 1.1f),
      .rot = r_float_r(0.0f, 3.14),
      .rot_vel = r_float_r(-0.5f, 0.5f),
      .alpha_t = r_float_r(0.0f, 2 * 3.14f),
      .alpha_vel = r_float_r(14.0f, 40.0f),
  };

  gs_add_object(gs, (SceneObject){st, &Star_SceneObject_Table});

  return st;
}

#endif