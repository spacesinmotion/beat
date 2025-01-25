#ifndef FARM_H
#define FARM_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include <assert.h>

typedef struct Farm {
  WorkProvider work_provider;

  int last_day_delivered;

  BuildingDisplay display;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->display.location.y); }

void fa_update(Farm *fa, GameScene *gs, Game *g, float dt) {

  bd_update(&fa->display, g);

  if (gs->day > fa->last_day_delivered) {
    fa->last_day_delivered = gs->day;
    while (fa->work_provider.clicks_done > 0 && gs->resource_pool.food + 1 <= gs->resource_pool_max.food) {
      gs->resource_pool.food++;
      fa->work_provider.clicks--;
      fa->work_provider.clicks_claimed--;
      fa->work_provider.clicks_work--;
      fa->work_provider.clicks_done--;
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
  // if (fa->temporary_deliver_timer > 0.0f)
  //   g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&fa->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Farm_table = {
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
};

Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .display = bd_create(g, (Recti){p.x, p.y, 4, 4}),
      .last_day_delivered = gs->day,
  };
  assert((void *)fa == (void *)&fa->work_provider);
  wp_init(&fa->work_provider, 3, 3, 6.0f);

  l_set_tileR(gs->level, fa->display.location, T_Farm);
  l_set_tile_contentR(gs->level, fa->display.location, to_TileContent(fa, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif