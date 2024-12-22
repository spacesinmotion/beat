#ifndef HOUSE_H
#define HOUSE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/assets.h"
#include "gc/gc.h"

typedef struct House {
  const sg_image *texture;
  const Buffer *buffer;
  Point location;
} House;

Color House_color() { return rgb(87, 163, 106); }

bool House_dead(House *h) {
  (void)h;
  return false;
}

Circle House_circle(House *h) {
  (void)h;
  return (Circle){0};
}

void House_update(House *h, Game *g, GameScene *gs, float dt) {
  (void)h;
  (void)g;
  (void)gs;
  (void)dt;
}

void House_draw(House *h, Game *g) {
  (void)g;
  g_noise(g, 0.0f);
  g_color(g, House_color());
  g_buffer(g, h->buffer, h->texture, Level_to_vecP(h->location));
}

static SceneObjectTable House_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)House_dead,
    .circle = (SceneObjectCircle)House_circle,
    .update = (SceneObjectUpdateCB)House_update,
    .draw = (SceneObjectDrawCB)House_draw,
};
House *House_init(Game *g, GameScene *gs, Point p) {
  House *w = gc_malloc(&gc, sizeof(House));
  *w = (House){
      .texture = g_image(g, Img_tilemap),
      .buffer = g_tilerect_buffer(g, 2, 2),
      .location = p,
  };

  Level_set_tileR(gs->level, (Recti){p.x, p.y, 2, 2}, T_House);
  GameScene_add_object(gs, (SceneObject){.context = w, &House_table});
  return w;
}
#endif // HOUSE_H