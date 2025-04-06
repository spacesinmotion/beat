#ifndef WOODCUTTER_H
#define WOODCUTTER_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Player.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct WoodCutter {
  BuildingDisplay display;
  int id;

  Player *player;

  int last_day_delivered;
  int manager_click_counter;
} WoodCutter;

Color wc_color() { return rgb(141, 100, 29); }
static inline Sizei wc_size() { return (Sizei){2, 2}; }

bool wc_dead(WoodCutter *wc) {
  (void)wc;
  return false;
}

float wc_render_order(WoodCutter *wc) { return l_to_y(wc->display.location.y); }

void wc_update(WoodCutter *wc, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&wc->display, g);

  if (gs->day > wc->last_day_delivered) {
    wc->last_day_delivered = gs->day;
    wc->manager_click_counter = 0;
  }
}

void wc_tick(WoodCutter *wc, GameScene *gs, Game *g, int tick) {
  (void)g;
  (void)gs;
  if (tick % 3 != 0)
    return;

  bd_flash(&wc->display);
  wc->player->resources.wood++;
}

void wc_draw(WoodCutter *wc, GameScene *gs, Game *g) {
  if (ri_contains(wc->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  WoodCutter (%d,%d,%d,%d)\n", wc->display.location.x, wc->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(wc->display.location));
  bd_draw(&wc->display, g, wc_color(), MI_WoodCutter);
}

void wc_to_json(CJHObject *o, WoodCutter *wc) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &wc->display);
  cjh_o_add_number(o, "last_day_delivered", wc->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", wc->manager_click_counter);
}

void wc_from_json(CJHObjectR *o, const char *key, WoodCutter *wc) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &wc->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable WoodCutter_table = {
    .type = "WoodCutter",
    .dead = (SceneObjectDeadCB)wc_dead,
    .render_order = (SceneObjectRenderOrderCB)wc_render_order,
    .tick = (SceneObjectTickCB)wc_tick,
    .update = (SceneObjectUpdateCB)wc_update,
    .draw = (SceneObjectDrawCB)wc_draw,
    .save = (SceneObjectSaveCB)wc_to_json,
};

Recti wc_location(const WoodCutter *wc) { return wc->display.location; }

static TileContentTable WoodCutter_TileContent_Table = {
    .location = (LocationCb)wc_location,
};

WoodCutter *WoodCutter_init(Game *g, GameScene *gs, Player *player, Point p) {
  const Sizei s = wc_size();
  WoodCutter *wc = g_malloc(sizeof(WoodCutter));
  *wc = (WoodCutter){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(wc),
      .player = player,
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, wc->display.location, to_TileContent(wc, &WoodCutter_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = wc, &WoodCutter_table});
  return wc;
}

#endif