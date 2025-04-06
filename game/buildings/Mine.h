#ifndef MINE_H
#define MINE_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct Mine {
  BuildingDisplay display;
  int id;

  Player *player;

  int last_day_delivered;
  int manager_click_counter;
} Mine;

Color mi_color() { return rgb(84, 78, 53); }
static inline Sizei mi_size() { return (Sizei){2, 2}; }

bool mi_dead(Mine *mi) {
  (void)mi;
  return false;
}

float mi_render_order(Mine *mi) { return l_to_y(mi->display.location.y); }

void mi_update(Mine *mi, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&mi->display, g);

  if (gs->day > mi->last_day_delivered) {
    mi->last_day_delivered = gs->day;
    mi->manager_click_counter = 0;
  }
}

void mi_tick(Mine *mi, GameScene *gs, Game *g, int tick) {
  (void)g;
  (void)gs;
  if (tick % 5 != 0)
    return;

  bd_flash(&mi->display);
  mi->player->resources.iron++;
}

void mi_draw(Mine *mi, GameScene *gs, Game *g) {
  if (ri_contains(mi->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Mine (%d,%d,%d,%d)\n", mi->display.location.x, mi->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(mi->display.location));
  bd_draw(&mi->display, g, mi_color(), MI_Mine);
}

void mi_to_json(CJHObject *o, Mine *mi) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &mi->display);
  cjh_o_add_number(o, "last_day_delivered", mi->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", mi->manager_click_counter);
}

void mi_from_json(CJHObjectR *o, const char *key, Mine *mi) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &mi->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Mine_table = {
    .type = "Mine",
    .dead = (SceneObjectDeadCB)mi_dead,
    .render_order = (SceneObjectRenderOrderCB)mi_render_order,
    .update = (SceneObjectUpdateCB)mi_update,
    .tick = (SceneObjectTickCB)mi_tick,
    .draw = (SceneObjectDrawCB)mi_draw,
    .save = (SceneObjectSaveCB)mi_to_json,
};

Recti mi_location(const Mine *mi) { return mi->display.location; }

static TileContentTable Mine_TileContent_Table = {
    .location = (LocationCb)mi_location,
};

Mine *Mine_init(Game *g, GameScene *gs, Player *player, Point p) {
  const Sizei s = mi_size();
  Mine *mi = g_malloc(sizeof(Mine));
  *mi = (Mine){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(mi),
      .player = player,
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, mi->display.location, to_TileContent(mi, &Mine_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mi, &Mine_table});
  return mi;
}

#endif