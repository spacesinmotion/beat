#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Circ.h"
#include "math/Vec2.h"

#include <float.h>

typedef struct Wearisome {
  const sg_image *texture;

  Color color;
  Vec2 position;
  Vec2 destination;
} Wearisome;

bool Wearisome_dead(Wearisome *w) {
  (void)w;
  return false;
}

Circle Wearisome_circle(Wearisome *w) { return (Circle){w->position, 8.0f}; }

void Wearisome_update(Wearisome *w, Game *g, GameScene *gs, float dt) {
  (void)g;

  Vec2 destination = w->position;
  float max_dist = DBL_MAX;
  Circle c_self = Wearisome_circle(w);
  for (int i = 0; i < gs->scene_objects.len; ++i) {
    if (w == gs->scene_objects.data[i].context)
      continue;

    Circle co = SceneObject_circle(&gs->scene_objects.data[i]);
    const float d = Circle_distance(c_self, co);
    if (d < max_dist) {
      max_dist = d;
      destination = co.center;
    }
  }

  w->position = v_lerp_about(w->position, destination, dt * 24.0f);
}

void Wearisome_draw(Wearisome *w, Game *g) {
  const float time = g_time(g);

  g_noise(g, 0.01f);
  g_color(g, (Color){0.01f * sin(time * 20) + 0.9f, 1.0f, 1.0f, 1.0f});
  Vec2 p = v_add(w->position, (Vec2){0, 5});
  g_object(g, g_animation_buffer(g), w->texture, p, g_frame(g) % 4);
}

SceneObjectTable Wearisome_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Wearisome_dead,
    .circle = (SceneObjectCircle)Wearisome_circle,
    .update = (SceneObjectUpdateCB)Wearisome_update,
    .draw = (SceneObjectDrawCB)Wearisome_draw,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, Vec2 pos, Color col) {
  Wearisome *w = gc_malloc(&gc, sizeof(Wearisome));
  *w = (Wearisome){
      .texture = g_image(g, Img_wearisome),
      .position = pos,
      .destination = pos,
      .color = col,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif