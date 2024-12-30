#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "math/Rect.h"

typedef struct Marketplace {
  G_Object buffer;
  Recti location;
} Marketplace;

Color Marketplace_color() { return rgb(196, 113, 65); }

bool Marketplace_dead(Marketplace *mp) {
  (void)mp;
  return false;
}

float Marketplace_render_order(Marketplace *mp) { return l_to_y(mp->location.y); }

void Marketplace_update(Marketplace *mp, GameScene *gs, float dt) {
  (void)mp;
  (void)gs;
  (void)dt;
}

void Marketplace_draw(Marketplace *mp, GameScene *gs, Game *g) {
  if (ri_contains((Recti){mp->location.x, mp->location.y, 2, 2}, gs->r.x, gs->r.y)) {
    c_printf(g, "##################\n");
    c_printf(g, "# Marketplace (%d,%d,%d,%d)\n", mp->location.x, mp->location.y, 4, 3);
    c_printf(g, "##################\n");
    // c_printf(g, "# water: %f\n", mp->resources.water);
    // c_printf(g, "#  food: %f\n", mp->resources.food);
    c_printf(g, "##################\n");
  }

  g_color(g, Marketplace_color());
  g_buffer(g, mp->buffer, Img_house_map, l_to_vecP(ri_bottom_right(mp->location)));
}

static SceneObjectTable Marketplace_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Marketplace_dead,
    .render_order = (SceneObjectRenderOrderCB)Marketplace_render_order,
    .update = (SceneObjectUpdateCB)Marketplace_update,
    .draw = (SceneObjectDrawCB)Marketplace_draw,
};
Marketplace *Marketplace_init(Game *g, GameScene *gs, Point p) {
  Marketplace *mp = g_malloc(g, sizeof(Marketplace));
  *mp = (Marketplace){
      .buffer = g_tilerect_buffer(g, 4, 3),
      .location = (Recti){p.x, p.y, 4, 3},
  };

  Level_set_tileR(gs->level, mp->location, T_Marketplace);
  gs_add_object(gs, (SceneObject){.context = mp, &Marketplace_table});
  return mp;
}
#endif // MARKETPLACE_H