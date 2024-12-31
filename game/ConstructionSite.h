#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"

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
    gs_construction_done(gs, g, cs->location, cs->key);
    return;
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cs->location));

  g_color(g, cs_color());
  // g_buffer(g, cs->buffer, Img_house_map, p);
  // g_color(g, white());
  for (int i = 0; i < cs->location.w; ++i)
    for (int j = 0; j < cs->location.h; ++j)
      g_object(g, g_animation_buffer(g), Img_marker, 0, v_add(p, l_to_vec(i, j)));

  // Point o[] = {
  //     {1, 3}, {2, 3}, {3, 3}, //
  //     {1, 2}, {2, 2}, {3, 2}, //
  //     {1, 1}, {2, 1}, {3, 1}, //
  // };
  int c = 0;
  for (int j = cs->location.h - 1; j >= 0; --j) {
    for (int i = 0; i < cs->location.w; ++i) {
      if (c < cs->clicks_done)
        g_color(g, rgb(107, 107, 107));
      else if (c < cs->clicks_work)
        g_color(g, rgb(101, 168, 110));
      else if (c < cs->clicks_claimed)
        g_color(g, rgb(89, 135, 146));
      else if (c < cs->clicks)
        g_color(g, rgb(255, 255, 255));
      else
        break;
      // g_color(g, rgb(255, 255, 255));
      g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, l_to_vec(i, j)), 0.75f);
      ++c;
    }
  }
  // g_color(g, rgb(255, 255, 255));
  // for (int i = cs->clicks; i < 1; ++i)
  //   g_objectS(g, g_animation_buffer(g), Img_wearisome, 13, v_add(p, l_to_vecP(o[i])), 0.75f);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, cs->key, p);
}

bool cs_has_work(ConstructionSite *cs, GameScene *gs) { return cs->clicks - cs->clicks_claimed > 0; }
void cs_claim_work(ConstructionSite *cs, GameScene *gs) { cs->clicks_claimed++; }
float cs_start_work(ConstructionSite *cs, GameScene *gs) {
  cs->clicks_work++;
  return 11.0;
}
void cs_done_work(ConstructionSite *cs, GameScene *gs) { cs->clicks_done++; }
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
    .has_work = (HasWorkCB)cs_has_work,
    .claim_work = (ClaimWorkCB)cs_claim_work,
    .start_work = (StartWorkCB)cs_start_work,
    .done_work = (DoneWorkCB)cs_done_work,
    .click = (ClickCB)cs_click,
};
ConstructionSite *ConstructionSite_init(Game *g, GameScene *gs, Recti r, int key) {
  ConstructionSite *cs = g_malloc(g, sizeof(ConstructionSite));
  *cs = (ConstructionSite){
      .location = r,
      .key = key,
  };

  l_set_tile_contentR(gs->level, cs->location, to_TileContent(cs, &ConstructionSite_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cs, &ConstructionSite_table});
  return cs;
}

#endif