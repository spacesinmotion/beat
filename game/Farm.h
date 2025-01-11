#ifndef FARM_H
#define FARM_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include <assert.h>

typedef struct Farm {
  WorkProvider work_provider;

  float temporary_deliver_timer;

  G_Object buffer;
  Recti location;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->location.y); }

void fa_update(Farm *fa, GameScene *gs, Game *g, float dt) {
  (void)g;

  if (fa->temporary_deliver_timer > 0.0f) {
    fa->temporary_deliver_timer -= dt;
    if (fa->temporary_deliver_timer <= 0.0f) {
      if (gs->resource_pool.food + 9 <= gs->resource_pool_max.food) {
        gs->resource_pool.food += 9;
        wp_reset(&fa->work_provider);
      } else
        fa->temporary_deliver_timer = 1.0f;
    }
  } else if (wp_is_done(&fa->work_provider)) {
    fa->temporary_deliver_timer = 5.0;
  }
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->location.x, fa->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(fa->location));
  g_color(g, fa_color());
  g_buffer(g, fa->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Food, p);
  if (fa->temporary_deliver_timer > 0.0f)
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
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .temporary_deliver_timer = 0.0f,
      .buffer = g_tilerect_buffer(g, 4, 4),
      .location = (Recti){p.x, p.y, 4, 4},
  };
  assert((void *)fa == (void *)&fa->work_provider);
  wp_init(&fa->work_provider, 3, 3, 8.0f);

  l_set_tileR(gs->level, fa->location, T_Farm);
  l_set_tile_contentR(gs->level, fa->location, to_TileContent(fa, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif