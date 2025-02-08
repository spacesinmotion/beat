#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include "game/jobs/DeliverJob.h"
#include "game/search/ResourceProviderSearch.h"
#include <assert.h>

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key);

typedef struct ConstructionSite {
  WorkProvider work_provider;

  Recti location;
  int key;
} ConstructionSite;

Color cs_color() { return rgb(43, 187, 223); }

bool cs_dead(ConstructionSite *cs) { return cs->work_provider.clicks_done > wp_fields(&cs->work_provider); }

float cs_render_order(ConstructionSite *cs) { return l_to_y(cs->location.y); }

void cs_update(ConstructionSite *cs, GameScene *gs, Game *g, float dt) {
  (void)dt;

  if (wp_is_done(&cs->work_provider)) {
    cs->work_provider.clicks_done++;
    l_clear_tileR(gs->level, cs->location);
    l_set_tile_contentR(gs->level, cs->location, NULL);
    gs_construction_done(gs, g, cs->location, cs->key);
  }
}

void cs_draw(ConstructionSite *cs, GameScene *gs, Game *g) {
  if (cs_dead(cs))
    return;

  if (ri_contains(cs->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionSite (%d,%d,%d,%d)\n", cs->location.x, cs->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cs->location));

  g_color(g, cs_color());
  for (int i = 0; i < cs->location.w; ++i)
    for (int j = 0; j < cs->location.h; ++j)
      g_object(g, g_animation_buffer(g), Img_marker, 0, v_add(p, l_to_vec(i, j)));

  wp_draw_click_fields(&cs->work_provider, g, p, true);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, cs->key, p);
}

static SceneObjectTable ConstructionSite_table = {
    .dead = (SceneObjectDeadCB)cs_dead,
    .render_order = (SceneObjectRenderOrderCB)cs_render_order,
    .update = (SceneObjectUpdateCB)cs_update,
    .draw = (SceneObjectDrawCB)cs_draw,
};

void cs_click(ConstructionSite *cs, GameScene *gs) {
  if (gs->resource_pool.construction_material > 0 && gs->clicks > 0) {
    if (cs->work_provider.clicks < wp_fields(&cs->work_provider) && gs->clicks > 0) {
      cs->work_provider.clicks++;
      gs->clicks--;
      // gs->resource_pool.construction_material--;
    }
  }
}

bool cs_take_constructionmaterial(ConstructionSite *cs, GameScene *gs) {
  (void)cs;
  gs->resource_pool.construction_material--;
  gs->resource_pool_claimed.construction_material--;
  return true;
}

void cs_get_constructionmaterial_done(ConstructionSite *cs, GameScene *gs) {
  (void)cs;
  (void)gs;
}

bool w_move_to_work(Wearisome *w, GameScene *gs, Recti location);
bool w_deliver_work(Wearisome *w, GameScene *gs, DeliverJob *job);
void cs_claim(ConstructionSite *cs, GameScene *gs, Wearisome *w, Resource r) {
  assert(r == R_Work);

  Recti marketplace = find_resource_building_rect(gs, cs->location, R_ConstructionMaterial);
  if (marketplace.w > 0) {
    gs->resource_pool_claimed.construction_material++;
    DeliverJob *job =
        deliver_job(marketplace, cs->location, MI_ConstructionMaterial, cs_color(), cs,
                    (CollectDoneCB)cs_take_constructionmaterial, (DeliverDoneCB)cs_get_constructionmaterial_done);
    if (w_deliver_work(w, gs, job))
      wp_claim(&cs->work_provider);
  }
}

static TileContentTable ConstructionSite_TileContent_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)cs_claim,
    .start = (StartCB)wp_start,
    .done = (DoneCB)wp_done,
    .click = (ClickCB)cs_click,
};

ConstructionSite *ConstructionSite_init(Game *g, GameScene *gs, Recti r, int key) {
  ConstructionSite *cs = g_malloc(g, sizeof(ConstructionSite));
  *cs = (ConstructionSite){
      .location = r,
      .key = key,
  };
  assert((void *)cs == (void *)&cs->work_provider);
  wp_init(&cs->work_provider, r.w, r.h, key == MI_Street ? 1.0f : 8.0f);

  l_set_tile_contentR(gs->level, cs->location, to_TileContent(cs, &ConstructionSite_TileContent_Table));
  l_set_tileR(gs->level, r, T_ConstructionSite);

  if (key == MI_Street)
    cs_click(cs, gs);
  else
    gs_loose_click(gs);

  gs_add_object(gs, (SceneObject){.context = cs, &ConstructionSite_table});
  return cs;
}

#endif