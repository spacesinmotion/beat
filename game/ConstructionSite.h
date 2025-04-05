#ifndef CONSTRUCTIONSITE_H
#define CONSTRUCTIONSITE_H

#include "extern/cjsonh/cjsonh.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/effects/Bling.h"
#include "math/Rect.h"
#include "math/random.h"

typedef struct ConstructionSite {
  Recti location;
  int id, key;
} ConstructionSite;

Color css_color() { return rgb(43, 187, 223); }

bool css_dead(ConstructionSite *css) { return css->location.h <= 0; }

float css_render_order(ConstructionSite *css) { return l_to_y(css->location.y); }

void gs_construction_done(GameScene *gs, Game *g, Recti r, int key);
void css_tick(ConstructionSite *css, GameScene *gs, Game *g, int tick) {
  Vec2 p = l_to_vecP(ri_bottom_right(css->location));
  Vec2 s = l_to_vec(css->location.w - 1, css->location.h - 1);
  p = v_add(p, (Vec2){r_float() * s.x, r_float() * s.y});
  Bling_init(gs, p, red());

  if (tick > css->location.h * css->location.w) {
    gs_construction_done(gs, g, css->location, css->key);
    css->location.h = 0;
  }
}
void css_update(ConstructionSite *css, GameScene *gs, Game *g, float dt) {
  (void)css;
  (void)gs;
  (void)g;
  (void)dt;

  // if (wp_is_done(&css->work_provider)) {
  //   css->work_provider.clicks_done++;
  //   l_set_tile_contentR(gs->level, css->location, NULL);
  //   gs_construction_done(gs, g, css->location, css->key);
  // }
}

void css_draw(ConstructionSite *css, GameScene *gs, Game *g) {
  if (css_dead(css))
    return;

  if (ri_contains(css->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionSite (%d,%d,%d,%d)\n", css->location.x, css->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(css->location));

  g_color(g, css_color());
  for (int i = 0; i < css->location.w; ++i)
    for (int j = 0; j < css->location.h; ++j)
      g_object(g, g_animation_buffer(g), Img_marker, 0, v_add(p, l_to_vec(i, j)));

  // wp_draw_click_fields(&css->work_provider, g, p, true);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, css->key, p);
}

void css_to_json(CJHObject *o, ConstructionSite *css) {
  cjh_o_add_number(o, "id", css->id);
  cjh_o_add_number(o, "key", css->key);
  cjh_o_add_array(o, "location", (CJHWriteArrayCB)ri_to_json, &css->location);
}

void css_from_json(CJHObjectR *o, const char *key, ConstructionSite *css) {

  if (streq(key, "last_day_delivered"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "manager_click_counter"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "location")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)ri_from_json, &css->location);
    indent -= 2;
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable ConstructionSite_table = {
    .type = "ConstructionSite",
    .dead = (SceneObjectDeadCB)css_dead,
    .render_order = (SceneObjectRenderOrderCB)css_render_order,
    .update = (SceneObjectUpdateCB)css_update,
    .tick = (SceneObjectTickCB)css_tick,
    .draw = (SceneObjectDrawCB)css_draw,
    .save = (SceneObjectSaveCB)css_to_json,
};

Recti css_location(const ConstructionSite *css) { return css->location; }

static TileContentTable ConstructionSite_TileContent_Table = {
    .location = (LocationCb)css_location,
};

ConstructionSite *ConstructionSite_init(GameScene *gs, Recti r, int key) {
  ConstructionSite *css = g_malloc(sizeof(ConstructionSite));
  *css = (ConstructionSite){
      .location = r,
      .id = unique_id(css),
      .key = key,
  };
  l_set_tile_contentR(gs->level, css->location, to_TileContent(css, &ConstructionSite_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = css, &ConstructionSite_table});
  return css;
}

#endif