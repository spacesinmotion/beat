#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Circ.h"
#include "math/Vec2.h"

typedef struct PathPoint {
  Vec2 p;
  struct PathPoint *next;
} PathPoint;

typedef struct Wearisome {
  const sg_image *texture;
  const sg_image *weapon;

  Vec2 position;
  Vec2 destination;

  PathPoint *path;
} Wearisome;

bool Wearisome_dead(Wearisome *w) {
  (void)w;
  return false;
}

Circle Wearisome_circle(Wearisome *w) { return (Circle){w->position, 8.0f}; }

void Wearisome_update(Wearisome *w, Game *g, GameScene *gs, float dt) {
  (void)g;
  if (w->path && v_eq(w->position, w->destination)) {
    w->destination = w->path->p;
    w->path = w->path->next;
  }
  w->position = v_lerp_about(w->position, w->destination, dt * 32.0);
}
void Wearisome_draw(Wearisome *w, Game *g) {
  g_noise(g, 0.01f);

  g_color(g, white());
  g_object(g, g_animation_buffer(g), w->weapon, v_add(w->position, (Vec2){8, 4}), 0.0f, 0);

  Vec2 p = v_add(w->position, (Vec2){0, 2});
  g_color(g, rgb(77, 213, 30));
  g_object(g, g_animation_buffer(g), w->texture, p, 0.0f, g_frame(g) % 4);
}

SceneObjectTable Wearisome_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Wearisome_dead,
    .circle = (SceneObjectCircle)Wearisome_circle,
    .update = (SceneObjectUpdateCB)Wearisome_update,
    .draw = (SceneObjectDrawCB)Wearisome_draw,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, Vec2 pos) {
  Wearisome *w = gc_malloc(&gc, sizeof(Wearisome));
  *w = (Wearisome){
      .texture = g_image(g, Img_wearisome),
      .weapon = g_image(g, Img_weapons),
      .position = pos,
      .destination = pos,
      .path = NULL,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif