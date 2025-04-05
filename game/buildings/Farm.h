#ifndef FARM_H
#define FARM_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct Farm {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }
static inline Sizei fa_size() { return (Sizei){2, 2}; }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->display.location.y); }

void fa_update(Farm *fa, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&fa->display, g);

  if (gs->day > fa->last_day_delivered) {
    fa->last_day_delivered = gs->day;
    fa->manager_click_counter = 0;
  }
}

void fa_tick(Farm *fa, GameScene *gs, Game *g, int tick) {
  (void)g;
  if (tick % 2 != 0)
    return;

  bd_flash(&fa->display);
  gs->resources.food++;
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->display.location.x, fa->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(fa->display.location));
  bd_draw(&fa->display, g, fa_color(), MI_Farm);
}

void fa_to_json(CJHObject *o, Farm *fa) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &fa->display);
  cjh_o_add_number(o, "last_day_delivered", fa->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", fa->manager_click_counter);
}

void fa_from_json(CJHObjectR *o, const char *key, Farm *fa) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &fa->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Farm_table = {
    .type = "Farm",
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .tick = (SceneObjectTickCB)fa_tick,
    .draw = (SceneObjectDrawCB)fa_draw,
    .save = (SceneObjectSaveCB)fa_to_json,
};

Recti fa_location(const Farm *fa) { return fa->display.location; }

static TileContentTable Farm_TileContent_Table = {
    .location = (LocationCb)fa_location,
};

Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = fa_size();
  Farm *fa = g_malloc(sizeof(Farm));
  *fa = (Farm){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(fa),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, fa->display.location, to_TileContent(fa, &Farm_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif