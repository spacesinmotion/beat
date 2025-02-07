#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include "game/BuildingDisplay.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "math/Rect.h"

typedef struct Marketplace {
  WorkProvider work_provider;
  BuildingDisplay display;

  int last_day_delivered;

} Marketplace;

static inline Color mp_color() { return rgb(196, 113, 65); }
static inline Sizei mp_size() { return (Sizei){4, 3}; }

bool mp_dead(Marketplace *mp) {
  (void)mp;
  return false;
}

float mp_render_order(Marketplace *mp) { return l_to_y(mp->display.location.y); }

void mp_update(Marketplace *mp, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)dt;

  if (gs->day > mp->last_day_delivered) {
    mp->last_day_delivered = gs->day;
    wp_reduce_clicks(&mp->work_provider, mp->work_provider.clicks_done);
  }
  bd_update(&mp->display, g);
}

void mp_draw(Marketplace *mp, GameScene *gs, Game *g) {
  if (ri_contains(mp->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Marketplace (%d,%d,%d,%d)\n", mp->display.location.x, mp->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&mp->display, g, mp_color(), MI_Marketplace);

  Vec2 p = l_to_vecP(ri_bottom_right(mp->display.location));
  wp_draw_click_fields(&mp->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable Marketplace_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)mp_dead,
    .render_order = (SceneObjectRenderOrderCB)mp_render_order,
    .draw = (SceneObjectDrawCB)mp_draw,
    .update = (SceneObjectUpdateCB)mp_update,
};

bool mp_provides(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;

  if (r == R_Water)
    return gs->resource_pool.water - gs->resource_pool_claimed.water > 0;
  else if (r == R_Food)
    return gs->resource_pool.food - gs->resource_pool_claimed.food > 0;
  else if (r == R_Work)
    return wp_provides(&mp->work_provider, gs, r);
  return false;
}

void mp_claim(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;
  if (r == R_Water)
    gs->resource_pool_claimed.water++;
  else if (r == R_Food)
    gs->resource_pool_claimed.food++;
  else if (r == R_Work)
    return wp_claim(&mp->work_provider, gs, r);
}

float mp_start(Marketplace *mp, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_start(&mp->work_provider, gs, r);

  return 0.1f;
}

void mp_done(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;
  if (r == R_Water) {
    gs->resource_pool_claimed.water--;
    gs->resource_pool.water--;
  } else if (r == R_Food) {
    gs->resource_pool_claimed.food--;
    gs->resource_pool.food--;
  } else if (r == R_Work)
    wp_done(&mp->work_provider, gs, r);
}

static TileContentTable Marketplace_TileContent_Table = {
    .provides = (ProvidesCB)mp_provides,
    .claim = (ClaimCB)mp_claim,
    .start = (StartCB)mp_start,
    .done = (DoneCB)mp_done,
    .click = (ClickCB)wp_click,
};

Marketplace *Marketplace_init(Game *g, GameScene *gs, Point p) {
  Sizei s = mp_size();
  Marketplace *mp = g_malloc(g, sizeof(Marketplace));
  *mp = (Marketplace){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .last_day_delivered = gs->day,
  };
  assert((void *)mp == (void *)&mp->work_provider);
  wp_init(&mp->work_provider, s.w - 1, s.h - 1, 0.1f);

  l_set_tileR(gs->level, mp->display.location, T_Marketplace);
  l_set_tile_contentR(gs->level, mp->display.location, to_TileContent(mp, &Marketplace_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mp, &Marketplace_table});
  return mp;
}

#endif // MARKETPLACE_H