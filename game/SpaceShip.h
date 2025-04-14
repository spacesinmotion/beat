#ifndef SPACESHIP_H
#define SPACESHIP_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Vec2.h"
#include "math/random.h"

typedef struct DriveStar {
  Vec2 pos;
  float scale, rot;
} DriveStar;

typedef struct SpaceShip {
  Vec2 center, pos, vel;

  DriveStar drive_star[32];
} SpaceShip;

bool sh_dead(const SpaceShip *sh) { return sh->pos.y < 0.0; }
float sh_render_order(const SpaceShip *sh) { return sh->pos.y; }

void sh_update(SpaceShip *sh, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)g;
  (void)dt;

  if (rand() % 250 < 4 && v_distance(sh->pos, sh->center) < 5)
    sh->vel = v_add(sh->vel, (Vec2){r_float_r(-1.5, 1.5f), r_float_r(-1.5, 1.5f)});
  else {
    Vec2 cor = v_sub(sh->pos, sh->center);
    sh->vel = v_add(sh->vel, v_mulf(cor, -dt * 0.5f));
  }

  sh->vel = v_mulf(sh->vel, 0.9f + 0.1f * gs->speed);
  sh->pos = v_add(sh->pos, v_mulf(sh->vel, dt));

  bool found_one = false;
  for (int i = 0; i < 32; ++i) {
    DriveStar *st = &sh->drive_star[i];
    st->scale *= r_float_r(0.85f, 0.96f);
    if (!found_one && st->scale < 0.1f) {
      found_one = true;
      st->pos.x = sh->pos.x - r_float_r(9.0f, 13.0f);
      st->pos.y = sh->pos.y - r_float_r(0.65f, 0.85f);
      st->rot = r_float_r(0.0f, 3.14f);
      st->scale = r_float_r(0.6f, 1.1f);
    } else {
      st->pos.x -= r_float_r(39.0f, 41.0f) * dt;
    }
  }
}

void sh_draw(SpaceShip *sh, GameScene *gs, Game *g) {
  (void)gs;

  for (int i = 0; i < 16; ++i) {
    const DriveStar *st = &sh->drive_star[i];
    g_color(g, alphaf(c_mix(red(), white(), st->scale * st->scale), st->scale * (0.5f + 0.1f * sin(st->scale))));
    g_objectRS(g, g_animation_buffer(g), Img_starship, 1, st->pos, st->rot, vec2f(st->scale));
  }
  g_color(g, white());
  g_objectS(g, g_animation_buffer(g), Img_starship, 0, sh->pos, vec2f(2.0f));
  // g_objectRS(g, g_animation_buffer(g), Img_starship, 1, sh->center, 0, 1.5);

  if (gs->health < 100.0) {
    Vec2 p = v_add(sh->pos, (Vec2){-6, 10});
    g_color(g, gray(150));
    g_objectS(g, g_rect_buffer(g), Img_starship, 15, p, (Vec2){20, 2});
    g_color(g, red());
    const float e = gs->health / 100.0f;
    g_objectS(g, g_rect_buffer(g), Img_starship, 15, p, (Vec2){e * 20, 2});
  }
}

SceneObjectTable SpaceShip_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)sh_dead,
    .render_order = (SceneObjectRenderOrderCB)sh_render_order,
    .update = (SceneObjectUpdateCB)sh_update,
    .draw = (SceneObjectDrawCB)sh_draw,
};

SpaceShip *SpaceShip_init(GameScene *gs, Vec2 pos) {
  SpaceShip *sh = (SpaceShip *)g_malloc(sizeof(SpaceShip));
  *sh = (SpaceShip){.center = pos, .pos = pos, .vel = (Vec2){0.0f, 0.0f}};
  gs_add_object(gs, (SceneObject){sh, &SpaceShip_SceneObject_Table});

  return sh;
}

#endif