#ifndef CLICKFACTORY_H
#define CLICKFACTORY_H

#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include <assert.h>

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
  for (int c = 0; c < 4; ++c) {
    if (c < cf->clicks_done)
      done_color(g);
    else if (c < cf->clicks_work)
      working_color(g);
    else if (c < cf->clicks_claimed)
      work_claimed_color(g);
    else if (c < cf->clicks)
      clicked_color(g);
    else
      break;
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, l_to_vecP(o[c])), 0.75f);
  }
  g_color(g, rgb(255, 255, 255));
  for (int i = cf->clicks; i < 4; ++i)
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 13, v_add(p, l_to_vecP(o[i])), 0.75f);
}

bool cf_provides(ClickFactory *cf, GameScene *gs, Resource r) {
  (void)gs;
  return r == R_Work && cf->clicks - cf->clicks_claimed > 0;
}
void cf_claim(ClickFactory *cf, GameScene *gs, Resource r) {
  (void)gs;

  assert(r == R_Work);
  cf->clicks_claimed++;
}
float cf_start(ClickFactory *cf, GameScene *gs, Resource r) {
  (void)gs;

  assert(r == R_Work);
  cf->clicks_work++;
  return 10.0;
}
void cf_done(ClickFactory *cf, GameScene *gs, Resource r) {

  assert(r == R_Work);
  cf->clicks_done++;
  if (cf->clicks_done == 4) {
    cf->clicks = cf->clicks_claimed = cf->clicks_work = cf->clicks_done = 0;
    gs_produce_click(gs);
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
    .provides = (ProvidesCB)cf_provides,
    .claim = (ClaimCB)cf_claim,
    .start = (StartCB)cf_start,
    .done = (DoneCB)cf_done,
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