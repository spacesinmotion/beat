#ifndef ENTERTAINMENT_H
#define ENTERTAINMENT_H

#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include <assert.h>

typedef struct Entertainment {
  G_Object buffer;
  Recti location;
  int claimed, started;
} Entertainment;

Color em_color() { return rgb(245, 99, 72); }

bool em_dead(Entertainment *em) {
  (void)em;
  return false;
}

float em_render_order(Entertainment *em) { return l_to_y(em->location.y); }

void em_update(Entertainment *em, GameScene *gs, float dt) {
  (void)em;
  (void)gs;
  (void)dt;
}

void em_draw(Entertainment *em, GameScene *gs, Game *g) {
  if (ri_contains(em->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Entertainment (%d,%d,%d,%d)\n", em->location.x, em->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(em->location));
  g_color(g, em_color());
  g_buffer(g, em->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, 6, p);

  Point o[] = {{1, 1}, {2, 1}};
  for (int i = 0; i < 2; ++i) {
    if (i < em->started)
      working_color(g);
    else if (i < em->claimed)
      work_claimed_color(g);
    else
      break;
    // g_color(g, rgb(255, 255, 255));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 14, v_add(p, l_to_vecP(o[i])), 0.75f);
  }
  g_color(g, rgb(255, 255, 255));
  for (int i = em->claimed; i < 2; ++i)
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 15, v_add(p, l_to_vecP(o[i])), 0.75f);
}

bool em_provides(Entertainment *em, GameScene *gs, Resource r) {
  (void)gs;
  return r == R_Entertainment && em->claimed < 2;
}
void em_claim(Entertainment *em, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Entertainment);

  em->claimed++;
}
float em_start(Entertainment *em, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Entertainment);

  em->started++;
  return 15.0;
}
void em_done(Entertainment *em, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Entertainment);

  em->started--;
  em->claimed--;
}

static SceneObjectTable Entertainment_table = {
    .dead = (SceneObjectDeadCB)em_dead,
    .render_order = (SceneObjectRenderOrderCB)em_render_order,
    .update = (SceneObjectUpdateCB)em_update,
    .draw = (SceneObjectDrawCB)em_draw,
};
static TileContentTable Entertainment_TileContent_Table = {
    .provides = (ProvidesCB)em_provides,
    .claim = (ClaimCB)em_claim,
    .start = (StartCB)em_start,
    .done = (DoneCB)em_done,
};
Entertainment *Entertainment_init(Game *g, GameScene *gs, Point p) {
  Entertainment *em = g_malloc(g, sizeof(Entertainment));
  *em = (Entertainment){
      .buffer = g_tilerect_buffer(g, 3, 2),
      .location = (Recti){p.x, p.y, 3, 2},
  };

  l_set_tileR(gs->level, em->location, T_Entertainment);
  l_set_tile_contentR(gs->level, em->location, to_TileContent(em, &Entertainment_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = em, &Entertainment_table});
  return em;
}

#endif