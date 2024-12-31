#ifndef CLICKFACTORY_H
#define CLICKFACTORY_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"

typedef struct ClickFactory {
  G_Object buffer;
  Recti location;
  int clicks, clicks_claimed, clicks_work, clicks_done;
} ClickFactory;

Color cf_color() { return rgb(255, 215, 0); }

bool cf_dead(ClickFactory *cf) {
  (void)cf;
  return false;
}

float cf_render_order(ClickFactory *cf) { return l_to_y(cf->location.y); }

void cf_update(ClickFactory *cf, GameScene *gs, float dt) {
  (void)cf;
  (void)gs;
  (void)dt;
}

void cf_draw(ClickFactory *cf, GameScene *gs, Game *g) {
  if (ri_contains(cf->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ClickFactory (%d,%d,%d,%d)\n", cf->location.x, cf->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cf->location));
  g_color(g, cf_color());
  g_buffer(g, cf->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, 5, p);

  Point o[] = {{1, 2}, {2, 2}, {1, 1}, {2, 1}};
  for (int i = 0; i < 4; ++i) {
    if (i < cf->clicks_done)
      g_color(g, rgb(107, 107, 107));
    else if (i < cf->clicks_work)
      g_color(g, rgb(101, 168, 110));
    else if (i < cf->clicks_claimed)
      g_color(g, rgb(89, 135, 146));
    else if (i < cf->clicks)
      g_color(g, rgb(255, 255, 255));
    else
      break;
    // g_color(g, rgb(255, 255, 255));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, l_to_vecP(o[i])), 0.75f);
  }
  g_color(g, rgb(255, 255, 255));
  for (int i = cf->clicks; i < 4; ++i)
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 13, v_add(p, l_to_vecP(o[i])), 0.75f);
}

bool cf_has_work(ClickFactory *cf, GameScene *gs) { return cf->clicks - cf->clicks_claimed > 0; }
void cf_claim_work(ClickFactory *cf, GameScene *gs) { cf->clicks_claimed++; }
float cf_start_work(ClickFactory *cf, GameScene *gs) {
  cf->clicks_work++;
  return 10.0;
}
void cf_done_work(ClickFactory *cf, GameScene *gs) {
  cf->clicks_done++;
  if (cf->clicks_done == 4) {
    cf->clicks = cf->clicks_claimed = cf->clicks_work = cf->clicks_done = 0;
    gs->clicks++;
  }
}
void cf_click(ClickFactory *cf, GameScene *gs) {
  (void)gs;
  if (cf->clicks < 4 && gs->clicks > 0) {
    cf->clicks++;
    gs->clicks--;
  }
}

static SceneObjectTable ClickFactory_table = {
    .dead = (SceneObjectDeadCB)cf_dead,
    .render_order = (SceneObjectRenderOrderCB)cf_render_order,
    .update = (SceneObjectUpdateCB)cf_update,
    .draw = (SceneObjectDrawCB)cf_draw,
};
static TileContentTable ClickFactory_TileContent_Table = {
    .has_work = (HasWorkCB)cf_has_work,
    .claim_work = (ClaimWorkCB)cf_claim_work,
    .start_work = (StartWorkCB)cf_start_work,
    .done_work = (DoneWorkCB)cf_done_work,
    .click = (ClickCB)cf_click,
};
ClickFactory *ClickFactory_init(Game *g, GameScene *gs, Point p) {
  ClickFactory *cf = g_malloc(g, sizeof(ClickFactory));
  *cf = (ClickFactory){
      .buffer = g_tilerect_buffer(g, 3, 3),
      .location = (Recti){p.x, p.y, 3, 3},
  };

  l_set_tileR(gs->level, cf->location, T_ClickFactory);
  l_set_tile_contentR(gs->level, cf->location, to_TileContent(cf, &ClickFactory_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cf, &ClickFactory_table});
  return cf;
}

#endif