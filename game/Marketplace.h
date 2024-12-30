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

Color mp_color() { return rgb(196, 113, 65); }

bool mp_dead(Marketplace *mp) {
  (void)mp;
  return false;
}

float mp_render_order(Marketplace *mp) { return l_to_y(mp->location.y); }

void mp_update(Marketplace *mp, GameScene *gs, float dt) {
  (void)mp;
  (void)gs;
  (void)dt;
}

void mp_draw(Marketplace *mp, GameScene *gs, Game *g) {
  if (ri_contains(mp->location, gs->r.x, gs->r.y)) {
    c_printf(g, "#####################\n");
    c_printf(g, "# Marketplace (%d,%d,%d,%d)\n", mp->location.x, mp->location.y, 4, 3);
    c_printf(g, "#####################\n");
  }

  g_color(g, mp_color());
  g_buffer(g, mp->buffer, Img_house_map, l_to_vecP(ri_bottom_right(mp->location)));
}

static SceneObjectTable Marketplace_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)mp_dead,
    .render_order = (SceneObjectRenderOrderCB)mp_render_order,
    .update = (SceneObjectUpdateCB)mp_update,
    .draw = (SceneObjectDrawCB)mp_draw,
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