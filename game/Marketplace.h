#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/assets.h"

typedef struct Marketplace {
  G_Object buffer;
  Point location;
} Marketplace;

Color Marketplace_color() { return rgb(196, 113, 65); }

bool Marketplace_dead(Marketplace *mp) {
  (void)mp;
  return false;
}

Circle Marketplace_circle(Marketplace *mp) {
  (void)mp;
  return (Circle){0};
}

void Marketplace_update(Marketplace *mp, Game *g, GameScene *gs, float dt) {
  (void)mp;
  (void)g;
  (void)gs;
  (void)dt;
}

void Marketplace_draw(Marketplace *mp, Game *g) {
  g_color(g, Marketplace_color());
  g_buffer(g, mp->buffer, Img_house_map, Level_to_vecP(mp->location));
}

static SceneObjectTable Marketplace_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Marketplace_dead,
    .circle = (SceneObjectCircle)Marketplace_circle,
    .update = (SceneObjectUpdateCB)Marketplace_update,
    .draw = (SceneObjectDrawCB)Marketplace_draw,
};
Marketplace *Marketplace_init(Game *g, GameScene *gs, Point p) {
  Marketplace *w = g_malloc(g, sizeof(Marketplace));
  *w = (Marketplace){
      .buffer = g_tilerect_buffer(g, 4, 3),
      .location = p,
  };

  Level_set_tileR(gs->level, (Recti){p.x, p.y, 4, 3}, T_Marketplace);
  GameScene_add_object(gs, (SceneObject){.context = w, &Marketplace_table});
  return w;
}
#endif // MARKETPLACE_H