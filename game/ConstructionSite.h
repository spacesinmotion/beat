#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include "extern/cjsonh/cjsonh.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "game/WorkProvider.h"
#include "math/Rect.h"

typedef struct ConstructionSite {
  WorkProvider work_provider;
  Recti location;
  int id, key;
} ConstructionSite;

Color cs_color() { return rgb(43, 187, 223); }

bool cs_dead(ConstructionSite *cs) { return cs->work_provider.clicks_done > wp_fields(&cs->work_provider); }

float cs_render_order(ConstructionSite *cs) { return l_to_y(cs->location.y); }

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key);
void cs_update(ConstructionSite *cs, GameScene *gs, Game *g, float dt) {
  (void)dt;

  if (wp_is_done(&cs->work_provider)) {
    cs->work_provider.clicks_done++;
    l_set_tile_contentR(gs->level, cs->location, NULL);
    gs_construction_done(gs, g, cs->location, cs->key);
  }
}

void cs_draw(ConstructionSite *cs, GameScene *gs, Game *g) {
  if (cs_dead(cs))
    return;

  if (ri_contains(cs->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionSite (%d,%d,%d,%d)\n", cs->location.x, cs->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cs->location));

  g_color(g, cs_color());
  for (int i = 0; i < cs->location.w; ++i)
    for (int j = 0; j < cs->location.h; ++j)
      g_object(g, g_animation_buffer(g), Img_marker, 0, v_add(p, l_to_vec(i, j)));

  wp_draw_click_fields(&cs->work_provider, g, p, true);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, cs->key, p);
}

void cs_to_json(CJHObject *o, ConstructionSite *cs) {
  cjh_o_add_number(o, "id", cs->id);
  cjh_o_add_number(o, "key", cs->key);
  cjh_o_add_object(o, "work_provider", (CJHWriteObjectCB)wp_to_json, &cs->work_provider);
  cjh_o_add_array(o, "location", (CJHWriteArrayCB)ri_to_json, &cs->location);
}

void cs_from_json(CJHObjectR *o, const char *key, ConstructionSite *cs) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "work_provider")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)wp_from_json, &cs->work_provider);
    indent -= 2;
  } else if (streq(key, "location")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)ri_from_json, &cs->location);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable ConstructionSite_table = {
    .type = "ConstructionSite",
    .dead = (SceneObjectDeadCB)cs_dead,
    .render_order = (SceneObjectRenderOrderCB)cs_render_order,
    .update = (SceneObjectUpdateCB)cs_update,
    .draw = (SceneObjectDrawCB)cs_draw,
    .save = (SceneObjectSaveCB)cs_to_json,
};

Recti cs_location(const ConstructionSite *cs) { return cs->location; }

void cs_click(ConstructionSite *cs, Point p, GameScene *gs) {
  (void)p;
  if (gs->resource_pool.construction_material > 0 && gs->clicks > 0) {
    if (cs->work_provider.clicks < wp_fields(&cs->work_provider) && gs->clicks > 0) {
      cs->work_provider.clicks++;
      gs->clicks--;
    }
  }
}

void w_earn_clicks(Wearisome *w, int c);
bool cs_work_construction_done(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ConstructionSite *cs = (ConstructionSite *)context;
  wp_done(&cs->work_provider, w);
  return false;
}
bool cs_work_start_construction(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;

  ConstructionSite *cs = (ConstructionSite *)context;
  wp_start(&cs->work_provider, w);
  w_deliver_clear(w);
  return w_queue_wait_for(w, cs->key == MI_Street ? 1.0f : 8.0f, QI(cs, cs_work_construction_done));
}
Color cmf_color();
bool cs_work_collect_construction_material(void *context, Wearisome *w, GameScene *gs) {
  ConstructionSite *cs = (ConstructionSite *)context;
  gs->resource_pool.construction_material--;
  gs->resource_pool_claimed.construction_material--;
  w_deliver(w, MI_ConstructionMaterial, cmf_color());
  return w_queue_move_to(w, gs, cs->location, QI(cs, cs_work_start_construction));
}

void cs_claim(ConstructionSite *cs, GameScene *gs, Wearisome *w, Resource r) {
  assert(r == R_Work);

  Recti marketplace = find_resource_building_rect(gs, cs->location, R_ConstructionMaterial);
  if (marketplace.w > 0) {
    gs->resource_pool_claimed.construction_material++;
    if (w_queue_move_to(w, gs, marketplace, QI(cs, cs_work_collect_construction_material)))
      wp_claim(&cs->work_provider);
  }
}

static TileContentTable ConstructionSite_TileContent_Table = {
    .location = (LocationCb)cs_location,
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)cs_claim,
    .click = (ClickCBx)cs_click,
};

ConstructionSite *ConstructionSite_init(GameScene *gs, Recti r, int key) {
  ConstructionSite *cs = g_malloc(sizeof(ConstructionSite));
  *cs = (ConstructionSite){
      .location = r,
      .id = unique_id(cs),
      .key = key,
  };
  assert((void *)cs == (void *)&cs->work_provider);
  wp_init(&cs->work_provider, r.w, r.h);

  l_set_tile_contentR(gs->level, cs->location, to_TileContent(cs, &ConstructionSite_TileContent_Table));

  gs_loose_click(gs);
  if (key == MI_Street)
    cs_click(cs, (Point){}, gs);

  gs_add_object(gs, (SceneObject){.context = cs, &ConstructionSite_table});
  return cs;
}

#endif