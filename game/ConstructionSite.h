#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include <assert.h>

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key);

typedef struct ConstructionSite {
  Recti location;
  int clicks, clicks_claimed, clicks_work, clicks_done, key;
} ConstructionSite;

Color cs_color() { return rgb(43, 187, 223); }

bool cs_dead(ConstructionSite *cs) { return cs->clicks_done > cs->location.w * cs->location.h; }

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

  if (cs->clicks_done == cs->location.w * cs->location.h) {
    cs->clicks_done++;
    l_clear_tileR(gs->level, cs->location);
    gs_construction_done(gs, g, cs->location, cs->key);
    return;
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cs->location));

  g_color(g, cs_color());
  for (int i = 0; i < cs->location.w; ++i)
    for (int j = 0; j < cs->location.h; ++j)
      g_object(g, g_animation_buffer(g), Img_marker, 0, v_add(p, l_to_vec(i, j)));

  int c = 0;
  for (int j = cs->location.h - 1; j >= 0; --j) {
    for (int i = 0; i < cs->location.w; ++i) {
      if (c < cs->clicks_done)
        done_color(g);
      else if (c < cs->clicks_work)
        working_color(g);
      else if (c < cs->clicks_claimed)
        work_claimed_color(g);
      else if (c < cs->clicks)
        clicked_color(g);
      else
        break;
      g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, l_to_vec(i, j)), 0.75f);
      ++c;
    }
  }
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, cs->key, p);
}

bool cs_provides(ConstructionSite *cs, GameScene *gs, Resource r) {
  (void)gs;
  return r == R_Work && cs->clicks - cs->clicks_claimed > 0;
}
void cs_claim(ConstructionSite *cs, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  cs->clicks_claimed++;
}
float cs_start(ConstructionSite *cs, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  cs->clicks_work++;
  if (cs->key == 0)
    return 2.0;
  return 11.0;
}
void cs_done(ConstructionSite *cs, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  cs->clicks_done++;
}
void cs_click(ConstructionSite *cs, GameScene *gs) {
  (void)gs;
  if (cs->clicks < cs->location.w * cs->location.h && gs->clicks > 0) {
    cs->clicks++;
    gs->clicks--;
  }
}

static SceneObjectTable ConstructionSite_table = {
    .dead = (SceneObjectDeadCB)cs_dead,
    .render_order = (SceneObjectRenderOrderCB)cs_render_order,
    .update = (SceneObjectUpdateCB)cs_update,
    .draw = (SceneObjectDrawCB)cs_draw,
};
static TileContentTable ConstructionSite_TileContent_Table = {
    .provides = (ProvidesCB)cs_provides,
    .claim = (ClaimCB)cs_claim,
    .start = (StartCB)cs_start,
    .done = (DoneCB)cs_done,
    .click = (ClickCB)cs_click,
};
ConstructionSite *ConstructionSite_init(Game *g, GameScene *gs, Recti r, int key) {
  ConstructionSite *cs = g_malloc(g, sizeof(ConstructionSite));
  *cs = (ConstructionSite){
      .location = r,
      .key = key,
  };

  l_set_tile_contentR(gs->level, cs->location, to_TileContent(cs, &ConstructionSite_TileContent_Table));
  l_set_tileR(gs->level, r, T_ConstructionSite);

  if (key == 0)
    cs_click(cs, gs);

  gs_add_object(gs, (SceneObject){.context = cs, &ConstructionSite_table});
  return cs;
}

#endif