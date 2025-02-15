#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include "game/BuildingDisplay.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/jobs/QueueItem.h"
#include "math/Rect.h"

#include <assert.h>

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
  else if (r == R_ConstructionMaterial)
    return gs->resource_pool.construction_material - gs->resource_pool_claimed.construction_material > 0;
  else if (r == R_Work)
    return wp_provides(&mp->work_provider, gs, r) &&
           find_resource_building(gs, mp->display.location, R_Deliver) != NULL && gs_free_storage(gs) > 0;

  return false;
}

bool mp_deliver_resource_done(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  switch (w->deliver_icon) {
  case MI_Water:
    gs->resource_pool.water += w->deliver_count;
    gs->storage_claimed -= w->deliver_count;
    break;
  case MI_Food:
    gs->resource_pool.food += w->deliver_count;
    gs->storage_claimed -= w->deliver_count;
    break;
  case MI_ConstructionMaterial:
    gs->resource_pool.construction_material += w->deliver_count;
    gs->storage_claimed -= w->deliver_count;
    break;
  case MI_Street:
  case MI_Marketplace:
  case MI_House:
  case MI_Click:
  case MI_Entertainment:
  case MI_IndustryOrResearch:
  case MI_Logistics:
  case MI_WareHouse:
  case Nb_MI:
    assert(false);
    break;
  }
  w_deliver_clear(w);

  Marketplace *mp = (Marketplace *)context;
  wp_done(&mp->work_provider, w);
  return false;
}

bool mp_collect_resource_done(void *context, Wearisome *w, GameScene *gs) {
  Marketplace *mp = (Marketplace *)context;
  w->need_mode = W_Normal;
  return w_queue_move_to(w, gs, mp->display.location, (QueueItem){mp, mp_deliver_resource_done});
}

void mp_claim(Marketplace *mp, GameScene *gs, Wearisome *w, Resource r) {
  (void)mp;
  if (r == R_Water)
    gs->resource_pool_claimed.water++;
  else if (r == R_Food)
    gs->resource_pool_claimed.food++;
  else if (r == R_ConstructionMaterial)
    gs->resource_pool_claimed.construction_material++;
  else if (r == R_Work) {
    TileContent *tc = find_resource_building(gs, mp->display.location, R_Deliver);
    if (tc) {
      tc_claim(tc, gs, w, R_Deliver);
      wp_claim(&mp->work_provider);
      w->queue_follow_up = (QueueItem){mp, mp_collect_resource_done};
    }
  }
}

static TileContentTable Marketplace_TileContent_Table = {
    .provides = (ProvidesCB)mp_provides,
    .claim = (ClaimCB)mp_claim,
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
  wp_init(&mp->work_provider, s.w - 1, s.h - 1);

  l_set_tileR(gs->level, mp->display.location, T_Marketplace);
  l_set_tile_contentR(gs->level, mp->display.location, to_TileContent(mp, &Marketplace_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mp, &Marketplace_table});
  return mp;
}

#endif // MARKETPLACE_H