#ifndef FARM_H
#define FARM_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include <assert.h>

typedef struct Farm {
  WorkProvider work_provider;

  G_Object buffer;
  Recti location;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }

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

  Vec2 p = l_to_vecP(ri_bottom_right(fa->location));
  g_color(g, fa_color());
  g_buffer(g, fa->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, 4, p);

  wp_draw_click_fields(&fa->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Farm_table = {
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
};

void fa_done(WorkProvider *wp, GameScene *gs, Resource r) {
  assert(r == R_Work);

  wp->clicks_done++;
  if (wp->clicks_done == 9) {
    wp->clicks = wp->clicks_claimed = wp->clicks_work = wp->clicks_done = 0;
    gs->resource_pool.food += 10;
  }
}

static TileContentTable Farm_TileContent_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)wp_claim,
    .start = (StartCB)wp_start,
    .done = (DoneCB)fa_done,
    .click = (ClickCB)wp_click,
};
Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .buffer = g_tilerect_buffer(g, 4, 4),
      .location = (Recti){p.x, p.y, 4, 4},
  };
  assert((void *)fa == (void *)&fa->work_provider);
  wp_init(&fa->work_provider, 3, 3, 8.0f);

  l_set_tileR(gs->level, fa->location, T_Farm);
  l_set_tile_contentR(gs->level, fa->location, to_TileContent(fa, &Farm_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif