#ifndef ALTAR_H
#define ALTAR_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct Altar {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} Altar;

Color al_color() { return rgb(127, 71, 155); }
static inline Sizei al_size() { return (Sizei){2, 2}; }

bool al_dead(Altar *al) {
  (void)al;
  return false;
}

float al_render_order(Altar *al) { return l_to_y(al->display.location.y); }

void al_update(Altar *al, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&al->display, g);

  if (gs->day > al->last_day_delivered) {
    al->last_day_delivered = gs->day;
    al->manager_click_counter = 0;
  }
}

void al_draw(Altar *al, GameScene *gs, Game *g) {
  if (ri_contains(al->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Altar (%d,%d,%d,%d)\n", al->display.location.x, al->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(al->display.location));
  bd_draw(&al->display, g, al_color(), MI_Altar);
}

void al_to_json(CJHObject *o, Altar *al) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &al->display);
  cjh_o_add_number(o, "last_day_delivered", al->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", al->manager_click_counter);
}

void al_from_json(CJHObjectR *o, const char *key, Altar *al) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &al->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Altar_table = {
    .type = "Altar",
    .dead = (SceneObjectDeadCB)al_dead,
    .render_order = (SceneObjectRenderOrderCB)al_render_order,
    .update = (SceneObjectUpdateCB)al_update,
    .draw = (SceneObjectDrawCB)al_draw,
    .save = (SceneObjectSaveCB)al_to_json,
};

Recti al_location(const Altar *al) { return al->display.location; }

static TileContentTable Altar_TileContent_Table = {
    .location = (LocationCb)al_location,
};

Altar *Altar_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = al_size();
  Altar *al = g_malloc(sizeof(Altar));
  *al = (Altar){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(al),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, al->display.location, to_TileContent(al, &Altar_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = al, &Altar_table});
  return al;
}

#endif