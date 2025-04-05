#ifndef FORGE_H
#define FORGE_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct Forge {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} Forge;

Color fo_color() { return rgb(49, 74, 71); }
static inline Sizei fo_size() { return (Sizei){2, 2}; }

bool fo_dead(Forge *fo) {
  (void)fo;
  return false;
}

float fo_render_order(Forge *fo) { return l_to_y(fo->display.location.y); }

void fo_update(Forge *fo, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&fo->display, g);

  if (gs->day > fo->last_day_delivered) {
    fo->last_day_delivered = gs->day;
    fo->manager_click_counter = 0;
  }
}

void fo_draw(Forge *fo, GameScene *gs, Game *g) {
  if (ri_contains(fo->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Forge (%d,%d,%d,%d)\n", fo->display.location.x, fo->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(fo->display.location));
  bd_draw(&fo->display, g, fo_color(), MI_Forge);
}

void fo_to_json(CJHObject *o, Forge *fo) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &fo->display);
  cjh_o_add_number(o, "last_day_delivered", fo->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", fo->manager_click_counter);
}

void fo_from_json(CJHObjectR *o, const char *key, Forge *fo) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &fo->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Forge_table = {
    .type = "Forge",
    .dead = (SceneObjectDeadCB)fo_dead,
    .render_order = (SceneObjectRenderOrderCB)fo_render_order,
    .update = (SceneObjectUpdateCB)fo_update,
    .draw = (SceneObjectDrawCB)fo_draw,
    .save = (SceneObjectSaveCB)fo_to_json,
};

Recti fo_location(const Forge *fo) { return fo->display.location; }

static TileContentTable Forge_TileContent_Table = {
    .location = (LocationCb)fo_location,
};

Forge *Forge_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = fo_size();
  Forge *fo = g_malloc(sizeof(Forge));
  *fo = (Forge){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(fo),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, fo->display.location, to_TileContent(fo, &Forge_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fo, &Forge_table});
  return fo;
}

#endif