#ifndef CASTLE_H
#define CASTLE_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"


typedef struct Castle {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} Castle;

Color cs_color() { return rgb(0, 80, 133); }
static inline Sizei cs_size() { return (Sizei){3, 3}; }

bool cs_dead(Castle *cs) {
  (void)cs;
  return false;
}

float cs_render_order(Castle *cs) { return l_to_y(cs->display.location.y); }

void cs_update(Castle *cs, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&cs->display, g);

  if (gs->day > cs->last_day_delivered) {
    cs->last_day_delivered = gs->day;
    cs->manager_click_counter = 0;
  }
}

void cs_draw(Castle *cs, GameScene *gs, Game *g) {
  if (ri_contains(cs->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Castle (%d,%d,%d,%d)\n", cs->display.location.x, cs->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(cs->display.location));
  bd_draw(&cs->display, g, cs_color(), MI_Castle);
}

void cs_to_json(CJHObject *o, Castle *cs) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &cs->display);
  cjh_o_add_number(o, "last_day_delivered", cs->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", cs->manager_click_counter);
}

void cs_from_json(CJHObjectR *o, const char *key, Castle *cs) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &cs->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Castle_table = {
    .type = "Castle",
    .dead = (SceneObjectDeadCB)cs_dead,
    .render_order = (SceneObjectRenderOrderCB)cs_render_order,
    .update = (SceneObjectUpdateCB)cs_update,
    .draw = (SceneObjectDrawCB)cs_draw,
    .save = (SceneObjectSaveCB)cs_to_json,
};

Recti cs_location(const Castle *cs) { return cs->display.location; }

static TileContentTable Castle_TileContent_Table = {
    .location = (LocationCb)cs_location,
};

Castle *Castle_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = cs_size();
  Castle *cs = g_malloc(sizeof(Castle));
  *cs = (Castle){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(cs),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, cs->display.location, to_TileContent(cs, &Castle_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = cs, &Castle_table});
  return cs;
}

#endif