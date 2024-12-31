#ifndef FARM_H
#define FARM_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"

typedef struct Farm {
  G_Object buffer;
  Recti location;
  int clicks;
  int clicks_claimed;
} Farm;

Color fa_color() { return rgb(11, 133, 0); }

bool fa_dead(Farm *fa) {
  (void)fa;
  return false;
}

float fa_render_order(Farm *fa) { return l_to_y(fa->location.y); }

void fa_update(Farm *fa, GameScene *gs, float dt) {
  (void)fa;
  (void)gs;
  (void)dt;
}

void fa_draw(Farm *fa, GameScene *gs, Game *g) {
  if (ri_contains(fa->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Farm (%d,%d,%d,%d)\n", fa->location.x, fa->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(fa->location));
  g_color(g, fa_color());
  g_buffer(g, fa->buffer, Img_house_map, p);

  g_color(g, rgb(255, 255, 255));
  for (int i = 0; i < fa->clicks; ++i) {
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){20, -2 + 4 * i}), 0.25);
  }
}

bool fa_has_work(Farm *fa, GameScene *gs) { return fa->clicks - fa->clicks_claimed > 0; }
void fa_claim_work(Farm *fa, GameScene *gs) { fa->clicks_claimed++; }
float fa_start_work(Farm *fa, GameScene *gs) {
  (void)fa;
  return 11.0;
}
void fa_done_work(Farm *fa, GameScene *gs) {
  fa->clicks_claimed--;
  fa->clicks--;
}
void fa_click(Farm *fa, GameScene *gs) {
  (void)gs;
  if (fa->clicks < 4 && gs->clicks > 0) {
    fa->clicks++;
    gs->clicks--;
  }
}

static SceneObjectTable Farm_table = {
    .dead = (SceneObjectDeadCB)fa_dead,
    .render_order = (SceneObjectRenderOrderCB)fa_render_order,
    .update = (SceneObjectUpdateCB)fa_update,
    .draw = (SceneObjectDrawCB)fa_draw,
};
static TileContentTable Farm_TileContent_Table = {
    .has_work = (HasWorkCB)fa_has_work,
    .claim_work = (ClaimWorkCB)fa_claim_work,
    .start_work = (StartWorkCB)fa_start_work,
    .done_work = (DoneWorkCB)fa_done_work,
    .click = (ClickCB)fa_click,
};
Farm *Farm_init(Game *g, GameScene *gs, Point p) {
  Farm *fa = g_malloc(g, sizeof(Farm));
  *fa = (Farm){
      .buffer = g_tilerect_buffer(g, 4, 4),
      .location = (Recti){p.x, p.y, 4, 4},
      .clicks = 4,
  };

  l_set_tileR(gs->level, fa->location, T_Farm);
  l_set_tile_contentR(gs->level, fa->location, to_TileContent(fa, &Farm_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif