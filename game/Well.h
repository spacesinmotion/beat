#ifndef WELL_H
#define WELL_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"

typedef struct Well {
  G_Object buffer;
  Recti location;
  int clicks, clicks_claimed, clicks_work, clicks_done;
} Well;

Color wl_color() { return rgb(0, 80, 133); }

bool wl_dead(Well *wl) {
  (void)wl;
  return false;
}

float wl_render_order(Well *wl) { return l_to_y(wl->location.y); }

void wl_update(Well *wl, GameScene *gs, float dt) {
  (void)wl;
  (void)gs;
  (void)dt;
}

void wl_draw(Well *wl, GameScene *gs, Game *g) {
  if (ri_contains(wl->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Well (%d,%d,%d,%d)\n", wl->location.x, wl->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(wl->location));
  g_color(g, wl_color());
  g_buffer(g, wl->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, 3, p);

  Point o[] = {{1, 2}, {1, 1}};
  for (int i = 0; i < 2; ++i) {
    if (i < wl->clicks_done)
      g_color(g, rgb(107, 107, 107));
    else if (i < wl->clicks_work)
      g_color(g, rgb(101, 168, 110));
    else if (i < wl->clicks_claimed)
      g_color(g, rgb(89, 135, 146));
    else if (i < wl->clicks)
      g_color(g, rgb(255, 255, 255));
    else
      break;
    // g_color(g, rgb(255, 255, 255));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, l_to_vecP(o[i])), 0.75f);
  }
  g_color(g, rgb(255, 255, 255));
  for (int i = wl->clicks; i < 2; ++i)
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 13, v_add(p, l_to_vecP(o[i])), 0.75f);
}

bool wl_has_work(Well *wl, GameScene *gs) {
  (void)gs;
  return wl->clicks - wl->clicks_claimed > 0;
}
void wl_claim_work(Well *wl, GameScene *gs) {
  (void)gs;
  wl->clicks_claimed++;
}
float wl_start_work(Well *wl, GameScene *gs) {
  (void)gs;
  wl->clicks_work++;
  return 11.0;
}
void wl_done_work(Well *wl, GameScene *gs) {
  wl->clicks_done++;
  if (wl->clicks_done == 2) {
    wl->clicks = wl->clicks_claimed = wl->clicks_work = wl->clicks_done = 0;
    gs->resource_pool.water += 4;
  }
}
void wl_click(Well *wl, GameScene *gs) {
  (void)gs;
  if (wl->clicks < 2 && gs->clicks > 0) {
    wl->clicks++;
    gs->clicks--;
  }
}

static SceneObjectTable Well_table = {
    .dead = (SceneObjectDeadCB)wl_dead,
    .render_order = (SceneObjectRenderOrderCB)wl_render_order,
    .update = (SceneObjectUpdateCB)wl_update,
    .draw = (SceneObjectDrawCB)wl_draw,
};
static TileContentTable Well_TileContent_Table = {
    .has_work = (HasWorkCB)wl_has_work,
    .claim_work = (ClaimWorkCB)wl_claim_work,
    .start_work = (StartWorkCB)wl_start_work,
    .done_work = (DoneWorkCB)wl_done_work,
    .click = (ClickCB)wl_click,
};
Well *Well_init(Game *g, GameScene *gs, Point p) {
  Well *wl = g_malloc(g, sizeof(Well));
  *wl = (Well){
      .buffer = g_tilerect_buffer(g, 2, 3),
      .location = (Recti){p.x, p.y, 2, 3},
  };

  l_set_tileR(gs->level, wl->location, T_Well);
  l_set_tile_contentR(gs->level, wl->location, to_TileContent(wl, &Well_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = wl, &Well_table});
  return wl;
}

#endif