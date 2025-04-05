#ifndef MENATARMS_H
#define MENATARMS_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"

typedef struct MenAtArms {
  BuildingDisplay display;

  int id;

  int last_day_delivered;
  int manager_click_counter;
} MenAtArms;

Color maa_color() { return rgb(78, 149, 142); }
static inline Sizei maa_size() { return (Sizei){3, 2}; }

bool maa_dead(MenAtArms *maa) {
  (void)maa;
  return false;
}

float maa_render_order(MenAtArms *maa) { return l_to_y(maa->display.location.y); }

void maa_update(MenAtArms *maa, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&maa->display, g);

  if (gs->day > maa->last_day_delivered) {
    maa->last_day_delivered = gs->day;
    maa->manager_click_counter = 0;
  }
}

void maa_draw(MenAtArms *maa, GameScene *gs, Game *g) {
  if (ri_contains(maa->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  MenAtArms (%d,%d,%d,%d)\n", maa->display.location.x, maa->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  // Vec2 p = l_to_vecP(ri_bottom_right(maa->display.location));
  bd_draw(&maa->display, g, maa_color(), MI_MenAtArms);
}

void maa_to_json(CJHObject *o, MenAtArms *maa) {
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &maa->display);
  cjh_o_add_number(o, "last_day_delivered", maa->last_day_delivered);
  cjh_o_add_number(o, "manager_click_counter", maa->manager_click_counter);
}

void maa_from_json(CJHObjectR *o, const char *key, MenAtArms *maa) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &maa->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable MenAtArms_table = {
    .type = "MenAtArms",
    .dead = (SceneObjectDeadCB)maa_dead,
    .render_order = (SceneObjectRenderOrderCB)maa_render_order,
    .update = (SceneObjectUpdateCB)maa_update,
    .draw = (SceneObjectDrawCB)maa_draw,
    .save = (SceneObjectSaveCB)maa_to_json,
};

Recti maa_location(const MenAtArms *maa) { return maa->display.location; }

static TileContentTable MenAtArms_TileContent_Table = {
    .location = (LocationCb)maa_location,
};

MenAtArms *MenAtArms_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = maa_size();
  MenAtArms *maa = g_malloc(sizeof(MenAtArms));
  *maa = (MenAtArms){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(maa),
      .last_day_delivered = gs->day,
      .manager_click_counter = 0,
  };

  l_set_tile_contentR(gs->level, maa->display.location, to_TileContent(maa, &MenAtArms_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = maa, &MenAtArms_table});
  return maa;
}

#endif