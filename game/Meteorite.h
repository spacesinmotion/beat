#ifndef METEORITE_H
#define METEORITE_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/SpaceShip.h"
#include "game/assets.h"
#include "game/effects/Bling.h"
#include "math/Color.h"
#include "math/Vec2.h"
#include "math/random.h"

typedef struct Meteorite {
  Vec2 pos, vel;
  float scale, rot, rot_vel, alpha_t, alpha_vel;
  bool hit_ship;
} Meteorite;

bool mt_dead(const Meteorite *mt) { return mt->hit_ship; }
float mt_render_order(const Meteorite *mt) { return mt->pos.y - 100; }

void mt_update(Meteorite *mt, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)g;
  (void)dt;

  const Vec2 aim = v_sub(gs->spaceship->pos, mt->pos);
  mt->vel.y += aim.y * 0.1f * dt;
  mt->vel.y *= 0.99f;
  Vec2 v = (Vec2){mt->vel.x * 0.25f + mt->vel.x * 0.75f * gs->speed, mt->vel.y};
  mt->pos = v_add(mt->pos, v_mulf(v, dt));
  mt->rot += mt->rot_vel * dt;
  mt->alpha_t += dt * mt->alpha_vel;

  mt->hit_ship = (v_distance(mt->pos, gs->spaceship->pos) < 10) || gs->initialized < 1.0f || gs->health <= 0.0f;
  if (mt->hit_ship) {
    Bling_init(gs, mt->pos, c_mix(yellow(), red(), r_float()));
    gs->health = f_max(-0.01f, gs->health - (mt->scale * gs->level * 3.0f));
    v = (Vec2){v.x, v.y * r_float_r(1.0f, 1.5f)};
    gs->spaceship->vel = v_add(gs->spaceship->vel, v_mulf(v, 0.5f));
    gs->speed *= 0.5f;
  }
  if (!mt->hit_ship && mt->pos.x < -100)
    mt->pos = (Vec2){600 + rand() % 200, 50 + rand() % 300};
}

void mt_draw(Meteorite *mt, GameScene *gs, Game *g) {
  (void)gs;

  d_color(g, alphaf(red(), 0.4f + 0.3f * sin(mt->alpha_t)));
  d_animation(g, Img_starship, 1, &(Transformation){mt->pos, mt->rot, vec2f(1.25f * mt->scale)});
  d_color(g, alphaf(white(), 0.7f + 0.1f * sin(mt->alpha_t)));
  d_animation(g, Img_starship, 2, &(Transformation){mt->pos, mt->rot, vec2f(mt->scale)});
}

SceneObjectTable Meteorite_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)mt_dead,
    .render_order = (SceneObjectRenderOrderCB)mt_render_order,
    .update = (SceneObjectUpdateCB)mt_update,
    .draw = (SceneObjectDrawCB)mt_draw,
};

Meteorite *Meteorite_init(GameScene *gs) {
  Meteorite *mt = (Meteorite *)g_malloc(sizeof(Meteorite));
  *mt = (Meteorite){
      .pos = (Vec2){500 + rand() % 200, 50 + rand() % 300},
      .vel = (Vec2){-(20 + rand() % 40), 0},
      .scale = r_float_r(1.5f, 2.75f),
      .rot = r_float_r(0.0f, 3.14),
      .rot_vel = r_float_r(0.75f, 1.5f) * (rand() % 2 == 0 ? 1 : -1),
      .alpha_t = r_float_r(0.0f, 2 * 3.14f),
      .alpha_vel = 3 * r_float_r(2.0f, 4.0f),
      .hit_ship = false,
  };

  gs_add_object(gs, (SceneObject){mt, &Meteorite_SceneObject_Table});

  return mt;
}

#endif