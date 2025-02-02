#ifndef FARM_H
#define FARM_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "math/Rect.h"
#include <assert.h>

typedef struct Farm {
  WorkProvider work_provider;

  int last_day_delivered;

  BuildingDisplay display;
} Farm;

static inline Color fa_color() { return rgb(11, 133, 0); }
static inline Sizei fa_size() { return (Sizei){3, 4}; }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->display.location.y); }

void fa_update(Farm *fa, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&fa->display, g);

  if (gs->day > fa->last_day_delivered) {
    fa->last_day_delivered = gs->day;
    if (wp_finish_production_cycle(&fa->work_provider, 8))
      bd_flash(&fa->display);
  }
  if (wp_has_something_stored(&fa->work_provider) && gs->daytime > 0.25 && gs->daytime < 0.26) {
    const int free_storage = gs_free_storage(gs);
    for (int i = 0; i < free_storage && fa->work_provider.storage > 0; ++i) {
      fa->work_provider.storage--;
      gs->resource_pool.food++;
    }
  }
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->display.location.x, fa->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&fa->display, g, fa_color(), MI_Food);
  Vec2 p = l_to_vecP(ri_bottom_right(fa->display.location));
  if (wp_has_something_stored(&fa->work_provider))
    g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&fa->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Farm_table = {
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
};

Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = fa_size();
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
  };
  assert((void *)fa == (void *)&fa->work_provider);
  wp_init(&fa->work_provider, s.w - 1, s.h - 1, 6.0f);

  l_set_tileR(gs->level, fa->display.location, T_Farm);
  l_set_tile_contentR(gs->level, fa->display.location, to_TileContent(fa, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif