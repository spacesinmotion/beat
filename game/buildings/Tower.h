#ifndef TOWER_H
#define TOWER_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct Tower {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} Tower;

Color to_color() { return rgb(86, 78, 71); }
static inline Sizei to_size() { return (Sizei){1, 1}; }

bool to_dead(Tower *to) {
  (void)to;
  return false;
}

float to_render_order(Tower *to) { return l_to_y(to->display.location.y); }

void to_update(Tower *to, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&to->display, g);

  if (gs->day > to->last_day_delivered) {
    to->last_day_delivered = gs->day;
    to->manager_click_counter = 0;
  }
}

void to_draw(Tower *to, GameScene *gs, Game *g) {
  if (ri_contains(to->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Tower (%d,%d,%d,%d)\n", to->display.location.x, to->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(to->display.location));
  bd_draw(&to->display, g, to_color(), MI_Tower);
}

void to_to_json(CJHObject *o, Tower *to) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &to->display);
  cjh_o_add_number(o, "last_day_delivered", to->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", to->manager_click_counter);
}

void to_from_json(CJHObjectR *o, const char *key, Tower *to) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &to->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Tower_table = {
    .type = "Tower",
    .dead = (SceneObjectDeadCB)to_dead,
    .render_order = (SceneObjectRenderOrderCB)to_render_order,
    .update = (SceneObjectUpdateCB)to_update,
    .draw = (SceneObjectDrawCB)to_draw,
    .save = (SceneObjectSaveCB)to_to_json,
};

Recti to_location(const Tower *to) { return to->display.location; }

static TileContentTable Tower_TileContent_Table = {
    .location = (LocationCb)to_location,
};

Tower *Tower_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = to_size();
  Tower *to = g_malloc(sizeof(Tower));
  *to = (Tower){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(to),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, to->display.location, to_TileContent(to, &Tower_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = to, &Tower_table});
  return to;
}

#endif