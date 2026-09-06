#ifndef MANAGER_H
#define MANAGER_H

#include "engine/SceneObject.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "game/BuildingDisplay.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "game/assets.h"

typedef struct Manager {
  WorkProvider work_provider;
  BuildingDisplay display;

  int id;

  int click_used;
  Resource used_manager_counter;

  G_Object click_used_text;
  int click_used_cache;
} Manager;

static inline Color mg_color() { return rgb(116, 31, 38); }
static inline Sizei mg_size() { return (Sizei){2, 2}; }

bool mg_dead(Manager *mg) {
  (void)mg;
  return false;
}

float mg_render_order(Manager *mg) { return l_to_y(mg->display.location.y); }

void mg_update(Manager *mg, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&mg->display, g);

  if (gs->a_new_day_just_started) {
    if (mg->work_provider.clicks_done > 0) {
      wp_reduce_clicks(&mg->work_provider, mg->work_provider.clicks_done);
      bd_flash(&mg->display);
    }
  }

  if (mg->click_used != mg->click_used_cache) {
    g_create_text(g, &mg->click_used_text, Oswald_Regular_12, str("%4.d", mg->click_used));
    mg->click_used_cache = mg->click_used;
  }
}

void mg_draw(Manager *mg, GameScene *gs, Game *g) {
  if (ri_contains(mg->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Manager (%d,%d,%d,%d)\n", mg->display.location.x, mg->display.location.y, mg->display.location.w,
             mg->display.location.h);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(mg->display.location));
  bd_draw(&mg->display, g, mg_color(), MI_Manager);

  wp_draw_click_fields(&mg->work_provider, g, v_add(p, l_to_vec(1, 1)), false);

  g_color(g, white());
  g_text(g, mg->click_used_text, Oswald_Regular_12, v_add(p, (Vec2){12, -5}));
}

void mg_to_json(CJHObject *o, Manager *mg) {
  cjh_o_add_object(o, "work_provider", (CJHWriteObjectCB)wp_to_json, &mg->work_provider);
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &mg->display);
  cjh_o_add_number(o, "click_used", mg->click_used);
  cjh_o_add_number(o, "used_manager_counter", mg->used_manager_counter - R_ManagerWork1 + 1);
}

void mg_from_json(CJHObjectR *o, const char *key, Manager *mg) {
  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "click_used"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "used_manager_counter"))
    printf("%.*s%s: %d\n", indent, space, key, (int)cjh_o_read_number(o) + R_ManagerWork1 - 1);

  else if (streq(key, "work_provider")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)wp_from_json, &mg->work_provider);
    indent -= 2;
  } else if (streq(key, "display")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)bd_from_json, &mg->display);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Manager_table = {
    .type = "Manager",
    .dead = (SceneObjectDeadCB)mg_dead,
    .render_order = (SceneObjectRenderOrderCB)mg_render_order,
    .update = (SceneObjectUpdateCB)mg_update,
    .draw = (SceneObjectDrawCB)mg_draw,
    .save = (SceneObjectSaveCB)mg_to_json,
};

Recti mg_location(const Manager *mg) { return mg->display.location; }

bool mg_provides(Manager *mg, GameScene *gs, Resource r) {
  if (r == R_Work)
    return wp_provides(&mg->work_provider, gs, r);
  return false;
}

bool mg_find_manager_work(void *context, Wearisome *w, GameScene *gs);
bool mg_click_building(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;

  TileContent *tc = l_contentP(gs->level, l_to_point(w->destination));
  if (tc) {
    tc_claim(tc, gs, w, mg->used_manager_counter);
    tc_click(tc, (Point){-1, -1}, gs);
    gs->clicks++; // clicks already taken
    mg->click_used--;
  }

  return w_leave_building(w, gs, QI(mg, mg_find_manager_work));
}

bool mg_find_manager_work(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;
  if (mg->click_used == 0) {
    wp_done(&mg->work_provider, w);
    return false;
  }

  mg->used_manager_counter = R_ManagerWork1;
  TileContent *provider = find_resource_building(gs, mg->display.location, mg->used_manager_counter);
  if (provider) {
    mg->used_manager_counter = R_ManagerWork2;
    provider = find_resource_building(gs, mg->display.location, mg->used_manager_counter);
  }
  if (provider) {
    mg->used_manager_counter = R_ManagerWork3;
    provider = find_resource_building(gs, mg->display.location, mg->used_manager_counter);
  }
  if (provider) {
    mg->used_manager_counter = R_ManagerWork4;
    provider = find_resource_building(gs, mg->display.location, mg->used_manager_counter);
  }

  if (provider && w_queue_move_to(w, gs, tc_location(provider), QI(mg, mg_click_building)))
    return true;
  return w_queue_wait_for(w, 0.25f, QI(mg, mg_find_manager_work));
}

bool mg_start_work(void *context, Wearisome *w, GameScene *gs) {
  Manager *mg = (Manager *)context;
  wp_start(&mg->work_provider, w);
  w->need_mode = W_Normal;
  mg->click_used = i_min(8, gs->clicks);
  gs->clicks -= mg->click_used;
  return mg_find_manager_work(mg, w, gs);
}

void mg_claim(Manager *mg, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Work) {
    if (w_queue_move_to(w, gs, mg->display.location, QI(mg, mg_start_work)))
      wp_claim(&mg->work_provider);
  }
}

static TileContentTable Manager_TileContent_Table = {
    .location = (LocationCb)mg_location,
    .provides = (ProvidesCB)mg_provides,
    .claim = (ClaimCB)mg_claim,
    .click = (ClickCBx)wp_click,
};

Manager *Manager_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = mg_size();
  Manager *mg = g_malloc(sizeof(Manager));
  *mg = (Manager){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(mg),
      .click_used = 0,
      .used_manager_counter = R_None,
      .click_used_cache = -1,
  };
  assert((void *)mg == (void *)&mg->work_provider);
  wp_init(&mg->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, mg->display.location, to_TileContent(mg, &Manager_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mg, &Manager_table});
  return mg;
}

#endif