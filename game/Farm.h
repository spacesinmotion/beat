#ifndef FARM_H
#define FARM_H

#include "game/GameScene.h"
#include "game/Level.h"

typedef struct Farm {
  G_Object buffer;
  Recti location;
} Farm;

Color fa_color() { return rgb(245, 222, 179); }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->location.y); }

void fa_update(Farm *fa, GameScene *gs, float dt) {
  (void)fa;
  (void)gs;
  (void)dt;
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->location.x, fa->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  g_color(g, fa_color());
  g_buffer(g, fa->buffer, Img_house_map, l_to_vecP(ri_bottom_right(fa->location)));
}

static SceneObjectTable Farm_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
};
Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .buffer = g_tilerect_buffer(g, 4, 4),
      .location = (Recti){p.x, p.y, 4, 4},
  };

  Level_set_tileR(gs->level, fa->location, T_Farm);
  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif