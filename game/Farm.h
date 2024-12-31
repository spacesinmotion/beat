#ifndef FARM_H
#define FARM_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"

typedef struct Farm {
  G_Object buffer;
  Recti location;
  int clicks, clicks_claimed, clicks_work, clicks_done;
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
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, 4, p);

  Point o[] = {
      {1, 3}, {2, 3}, {3, 3}, //
      {1, 2}, {2, 2}, {3, 2}, //
      {1, 1}, {2, 1}, {3, 1}, //
  };
  for (int i = 0; i < 9; ++i) {
    if (i < fa->clicks_done)
      g_color(g, rgb(107, 107, 107));
    else if (i < fa->clicks_work)
      g_color(g, rgb(101, 168, 110));
    else if (i < fa->clicks_claimed)
      g_color(g, rgb(89, 135, 146));
    else if (i < fa->clicks)
      g_color(g, rgb(255, 255, 255));
    else
      break;
    // g_color(g, rgb(255, 255, 255));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, l_to_vecP(o[i])), 0.75f);
  }
  g_color(g, rgb(255, 255, 255));
  for (int i = fa->clicks; i < 9; ++i)
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 13, v_add(p, l_to_vecP(o[i])), 0.75f);
}

bool fa_has_work(Farm *fa, GameScene *gs) { return fa->clicks - fa->clicks_claimed > 0; }
void fa_claim_work(Farm *fa, GameScene *gs) { fa->clicks_claimed++; }
float fa_start_work(Farm *fa, GameScene *gs) {
  fa->clicks_work++;
  return 11.0;
}
void fa_done_work(Farm *fa, GameScene *gs) {
  fa->clicks_done++;
  if (fa->clicks_done == 9) {
    fa->clicks = fa->clicks_claimed = fa->clicks_work = fa->clicks_done = 0;
    gs->resources.food += 10;
  }
}
void fa_click(Farm *fa, GameScene *gs) {
  (void)gs;
  if (fa->clicks < 9 && gs->clicks > 0) {
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
      .clicks = 9,
  };

  l_set_tileR(gs->level, fa->location, T_Farm);
  l_set_tile_contentR(gs->level, fa->location, to_TileContent(fa, &Farm_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = fa, &Farm_table});
  return fa;
}

#endif