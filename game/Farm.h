#ifndef FARM_H
#define FARM_H

#include "game/BuildingDisplay.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"

typedef struct Farm {
  WorkProvider work_provider;
  BuildingDisplay display;

  int id;

  int manager_click_counter;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }
static inline Sizei fa_size() { return (Sizei){3, 4}; }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->display.location.y); }

void fa_update(Farm *fa, GameScene *gs, Game *g, float dt) {
  (void)dt;
  bd_update(&fa->display, g);

  gs->resource_pool_spread.food += fa->work_provider.local_storage;

  if (gs->a_new_day_just_started) {
    fa->manager_click_counter = 0;
    wp_clear_done_work(&fa->work_provider);
  }
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->display.location.x, fa->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
    c_printf(g, " %10s: %d\n", "storage", fa->work_provider.local_storage);
    c_printf(g, " %10s: %d\n", "taken", wp_storage_taken(&fa->work_provider));
    c_printf(g, "----------------------\n");
  }

  bd_draw(&fa->display, g, fa_color(), MI_Food);

  Vec2 p = l_to_vecP(ri_bottom_right(fa->display.location));
  wp_draw_click_fields(&fa->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
  wp_draw_storage(&fa->work_provider, g, v_add(p, l_to_vec(0, 1)));
}

void fa_to_json(CJHObject *o, Farm *fa) {
  cjh_o_add_object(o, "work_provider", (CJHWriteObjectCB)wp_to_json, &fa->work_provider);
  cjh_o_add_object(o, "display", (CJHWriteObjectCB)bd_to_json, &fa->display);
  cjh_o_add_number(o, "manager_click_counter", fa->manager_click_counter);
}

static SceneObjectTable Farm_table = {
    .type = "Farm",
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
    .save = (SceneObjectSaveCB)fa_to_json,
};

Recti fa_location(const Farm *fa) { return fa->display.location; }

bool fa_provides(Farm *fa, GameScene *gs, Resource r) {
  if (r == R_Food)
    return wp_has_something_to_deliver(&fa->work_provider);
  if (r == R_Work)
    return wp_provides(&fa->work_provider, gs, r);
  if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    return (int)r > fa->manager_click_counter && wp_has_work(&fa->work_provider);
  return r == R_Deliver && wp_has_something_to_deliver(&fa->work_provider);
}

bool fa_done_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Farm *fa = (Farm *)context;
  wp_done_and_store(&fa->work_provider, w);
  return false;
}
bool fa_start_work(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Farm *fa = (Farm *)context;
  wp_start(&fa->work_provider, w);
  return w_queue_wait_for(w, 6.0f, QI(fa, fa_done_work));
}

bool fa_collect_storage(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  Farm *fa = (Farm *)context;
  w_deliver(w, MI_Food, fa_color());
  if (qi_on_done(&w->queue_follow_up, w, gs))
    return wp_deliver_taken(&fa->work_provider);
  return false;
}

void fa_claim(Farm *fa, GameScene *gs, Wearisome *w, Resource r) {
  if (r == R_Food) {
    fa->work_provider.local_storage_claimed++;
  } else if (r == R_Work) {
    if (w_queue_move_to(w, gs, fa->display.location, QI(fa, fa_start_work)))
      wp_claim(&fa->work_provider);
  } else if (r == R_Deliver) {
    if (w_queue_move_to(w, gs, fa->display.location, QI(fa, fa_collect_storage)))
      wp_claim_deliver(&fa->work_provider, gs);
  } else if (r >= R_ManagerWork1 && r <= R_ManagerWork4)
    fa->manager_click_counter = r;
}

void fa_take(Farm *fa, GameScene *gs, Resource r, bool pay) {
  assert(r == R_Food);

  if (r == R_Food) {
    fa->work_provider.local_storage--;
    fa->work_provider.local_storage_claimed--;

    if (pay) {
      gs->clicks++;
      bd_flash(&fa->display);
      CoinAnimation_init(gs, bd_gain_something_location(&fa->display));
    }
  }
}

void fa_from_json(CJHObjectR *o, const char *key, Farm *fa) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "work_provider")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)wp_from_json, &fa->work_provider);
    indent -= 2;
  } else if (streq(key, "display")) {
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

static TileContentTable Farm_TileContent_Table = {
    .location = (LocationCb)fa_location,
    .provides = (ProvidesCB)fa_provides,
    .claim = (ClaimCB)fa_claim,
    .take = (TakeCB)fa_take,
    .click = (ClickCBx)wp_click_with_storage,
};

Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = fa_size();
  Farm *fa = g_malloc(sizeof(Farm));
  *fa = (Farm){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.h}),
      .id = unique_id(fa),
      .manager_click_counter = 0,
  };
  assert((void *)fa == (void *)&fa->work_provider);
  wp_init(&fa->work_provider, s.w - 1, s.h - 1);

  l_set_tile_contentR(gs->level, fa->display.location, to_TileContent(fa, &Farm_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif