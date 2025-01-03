#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
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

void cs_update(ConstructionSite *cs, GameScene *gs, float dt) {
  (void)cs;
  (void)gs;
  (void)dt;
}

void cs_draw(ConstructionSite *cs, GameScene *gs, Game *g) {
  if (ri_contains(cs->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionSite (%d,%d,%d,%d)\n", cs->location.x, cs->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  if (cs->work_provider.clicks_done == wp_fields(&cs->work_provider)) {
    cs->work_provider.clicks_done++;
    l_clear_tileR(gs->level, cs->location);
    gs_construction_done(gs, g, cs->location, cs->key);
    return;
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

void cs_done(WorkProvider *wp, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  wp->clicks_done++;
}

static TileContentTable ConstructionSite_TileContent_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)wp_claim,
    .start = (StartCB)wp_start,
    .done = (DoneCB)cs_done,
    .click = (ClickCB)wp_click,
};

ConstructionSite *ConstructionSite_init(Game *g, GameScene *gs, Recti r, int key) {
  ConstructionSite *cs = g_malloc(g, sizeof(ConstructionSite));
  *cs = (ConstructionSite){
      .location = r,
      .key = key,
  };
  assert((void *)cs == (void *)&cs->work_provider);
  wp_init(&cs->work_provider, r.w, r.h, key == 0 ? 2.0f : 11.0f);

  l_set_tile_contentR(gs->level, cs->location, to_TileContent(cs, &ConstructionSite_TileContent_Table));
  l_set_tileR(gs->level, r, T_ConstructionSite);

  if (key == 0)
    wp_click(&cs->work_provider, gs);

  gs_add_object(gs, (SceneObject){.context = cs, &ConstructionSite_table});
  return cs;
}

#endif