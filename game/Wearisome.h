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

#include <float.h>
#include <stdlib.h>

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
    const float d = v_distance(dest, w->position);
    if (d > 6 + 8 + 8) {
      w->position = v_lerp_about(w->position, dest, dt * 32.0f);

    } else if (d < 8 + 8 + 3) {
      w->position = v_lerp_about(w->position, dest, -dt * 24.0f);

    } else if (w->current_action_time > 0.0) {
      w->current_action_time -= dt;

      if (w->current_action != Stay) {
        Vec2 v = v_sub(dest, w->position);
        v = (Vec2){-v.y, v.x};
        const float dir = w->current_action == RotateLeft ? 1.0f : -1.0f;
        w->position = v_lerp_about(w->position, v_add(w->position, v), dir * dt * 8.0f);
      }

    } else {
      w->current_action_time = 0.5f + ((float)(rand() / (float)RAND_MAX)) * 2.5f;
      const int r = rand() % 1000;
      if (r < 200)
        w->current_action = Stay;
      else if (r < 600)
        w->current_action = RotateLeft;
      else
        w->current_action = RotateRight;
    }
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
      .current_action_time = 0.0f,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif