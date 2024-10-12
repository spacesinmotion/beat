#ifndef WEARISOME
#define WEARISOME

#include "SceneObject.h"
#include "gc/gc.h"
#include <math.h>

#include "GameScene.h"

typedef struct Wearisome {
  int frame;
} Wearisome;

bool Wearisome_dead(Wearisome *w) {
  (void)w;
  return false;
}

void Wearisome_draw(Wearisome *w, Game *g) {
  float time = Game_time(g);
  const int frame = (int)(time * 4);
  if (frame % 32 > 29)
    w->frame = 1;
  else
    w->frame = 0;

  d_noise(g, 0.1f);
  d_color(g, 0.01f * sin(time * 20) + 0.9f, 1.0f, 1.0f, 1.0f);
  d_object(g, d_animation_buffer(g), d_animation_image(g), (Vec2){16 + (int)(16 * sin(time)), 0.0f}, w->frame);
}

SceneObjectTable Wearisome_table = (SceneObjectTable){
    .dead = (bool (*)(SceneObject *))Wearisome_dead,
    .update = NULL,
    .draw = (void (*)(SceneObject *, Game *))Wearisome_draw,
};

void Wearisome_init(Game *g, GameScene *gs) {
  Wearisome *w = gc_malloc(&gc, sizeof(Wearisome));
  *w = (Wearisome){0};
  GameScene_add_object(gs, (SceneObject){w, &Wearisome_table});
}

#endif