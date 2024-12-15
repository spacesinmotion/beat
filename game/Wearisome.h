#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math.h"
#include "math/Circ.h"
#include "math/Vec2.h"
#include "math/random.h"

#include <float.h>

typedef enum CurrentAction {
  Stay,
  RotateLeft,
  RotateRight,
} CurrentAction;

typedef struct Wearisome {
  const sg_image *texture;
  const sg_image *weapon;

  Faction faction;
  Vec2 position;
  Vec2 destination;

  SceneObject enemy;

  CurrentAction current_action;
  float current_action_time;
  float current_enemy_dist;
  float current_speed;

} Wearisome;

bool Wearisome_dead(Wearisome *w) {
  (void)w;
  return false;
}

Circle Wearisome_circle(Wearisome *w) { return (Circle){w->position, 8.0f}; }
Faction Wearisome_faction(Wearisome *w) { return w->faction; }

void Wearisome_update(Wearisome *w, Game *g, GameScene *gs, float dt) {
  (void)g;

  if (SceneObject_dead(&w->enemy)) {
    float max_dist = DBL_MAX;
    Circle c_self = Wearisome_circle(w);
    for (int i = 0; i < gs->scene_objects.len; ++i) {
      if (w == gs->scene_objects.data[i].context)
        continue;

      Circle co = SceneObject_circle(&gs->scene_objects.data[i]);
      const float d = Circle_distance(c_self, co);
      if (d < max_dist) {
        max_dist = d;
        w->enemy = gs->scene_objects.data[i];
      }
    }
  }

  if (!SceneObject_dead(&w->enemy)) {
    Vec2 dest = SceneObject_circle(&w->enemy).center;
    Vec2 v_dest = v_normalized(v_sub(dest, w->position));

    const float d = v_distance(dest, w->position);
    float speed = w->current_speed;
    Vec2 v_d = (Vec2){};
    if (d > 8 + 8 + w->current_enemy_dist + 4) {
      v_d = v_mulf(v_dest, 32.0f);
    } else if (d < 8 + 8 + w->current_enemy_dist - 4) {
      v_d = v_mulf(v_dest, -32.0f);
    } else
      speed /= 4.0f;

    Vec2 v_m = (Vec2){};
    if (w->current_action_time > 0.0) {
      w->current_action_time -= dt;

      if (w->current_action != Stay) {
        Vec2 v = (Vec2){-v_dest.y, v_dest.x};
        const float dir = w->current_action == RotateLeft ? 1.0f : -1.0f;
        v_m = v_mulf(v, dir * 8.0f);
      }

    } else {
      w->current_action_time = r_float_r(0.5f, 3.0f);
      w->current_enemy_dist = r_float_r(4.0f, 12.0f);
      w->current_speed = r_float_r(24.0f, 34.0f);

      const int r = rand() % 1000;
      if (r < 200)
        w->current_action = Stay;
      else if (r < 600)
        w->current_action = RotateLeft;
      else
        w->current_action = RotateRight;
    }
    w->position = v_lerp_about(w->position, v_add(w->position, v_add(v_d, v_m)), dt * speed);
  }
}

void Wearisome_draw(Wearisome *w, Game *g) {
  g_noise(g, 0.01f);
  g_color(g, (Color){1, 1, 1, 1});
  if (!SceneObject_dead(&w->enemy)) {
    Vec2 dest = SceneObject_circle(&w->enemy).center;
    Vec2 v = v_normalized(v_sub(dest, w->position));
    float rot = atan2(v.x, v.y);
    v = v_mulf(v, 8);
    v = v_add(v, (Vec2){0, 2});
    g_object(g, g_animation_buffer(g), w->weapon, v_add(w->position, v), rot, 0);
  } else
    g_object(g, g_animation_buffer(g), w->weapon, v_add(w->position, (Vec2){8, 4}), 0.0f, 0);

  Vec2 p = v_add(w->position, (Vec2){0, 2});
  g_color(g, w->faction == Evil ? rgb(231, 69, 38) : rgb(77, 213, 30));
  g_object(g, g_animation_buffer(g), w->texture, p, 0.0f, g_frame(g) % 4);
}

SceneObjectTable Wearisome_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Wearisome_dead,
    .circle = (SceneObjectCircle)Wearisome_circle,
    .update = (SceneObjectUpdateCB)Wearisome_update,
    .draw = (SceneObjectDrawCB)Wearisome_draw,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, Vec2 pos, Faction f) {
  Wearisome *w = gc_malloc(&gc, sizeof(Wearisome));
  *w = (Wearisome){
      .texture = g_image(g, Img_wearisome),
      .weapon = g_image(g, Img_weapons),
      .position = pos,
      .destination = pos,
      .faction = f,
      .enemy = (SceneObject){NULL, NULL},
      .current_enemy_dist = 8,
      .current_speed = 32.0f,
      .current_action_time = 0.0f,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif