#ifndef ARCHERS_H
#define ARCHERS_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct Archers {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} Archers;

Color ar_color() { return rgb(192, 108, 44); }
static inline Sizei ar_size() { return (Sizei){2, 3}; }

bool ar_dead(Archers *ar) {
  (void)ar;
  return false;
}

float ar_render_order(Archers *ar) { return l_to_y(ar->display.location.y); }

void ar_update(Archers *ar, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&ar->display, g);

  if (gs->day > ar->last_day_delivered) {
    ar->last_day_delivered = gs->day;
    ar->manager_click_counter = 0;
  }
}

void ar_draw(Archers *ar, GameScene *gs, Game *g) {
  if (ri_contains(ar->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Archers (%d,%d,%d,%d)\n", ar->display.location.x, ar->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(ar->display.location));
  bd_draw(&ar->display, g, ar_color(), MI_Archers);
}

void ar_to_json(CJHObject *o, Archers *ar) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &ar->display);
  cjh_o_add_number(o, "last_day_delivered", ar->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", ar->manager_click_counter);
}

void ar_from_json(CJHObjectR *o, const char *key, Archers *ar) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &ar->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Archers_table = {
    .type = "Archers",
    .dead = (SceneObjectDeadCB)ar_dead,
    .render_order = (SceneObjectRenderOrderCB)ar_render_order,
    .update = (SceneObjectUpdateCB)ar_update,
    .draw = (SceneObjectDrawCB)ar_draw,
    .save = (SceneObjectSaveCB)ar_to_json,
};

Recti ar_location(const Archers *ar) { return ar->display.location; }

static TileContentTable Archers_TileContent_Table = {
    .location = (LocationCb)ar_location,
};

Archers *Archers_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = ar_size();
  Archers *ar = g_malloc(sizeof(Archers));
  *ar = (Archers){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(ar),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, ar->display.location, to_TileContent(ar, &Archers_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = ar, &Archers_table});
  return ar;
}

#endif