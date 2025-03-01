#ifndef ENTERTAINMENT_H
#define ENTERTAINMENT_H

#include "game/BuildingDisplay.h"
#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/House.h"
#include "game/TileContent.h"
#include "game/Wearisome.h"
#include "math/Rect.h"

#include <assert.h>

typedef struct Entertainment {
  BuildingDisplay display;

  int claimed, started;
  bool some_one_is_done;
} Entertainment;

static inline Color em_color() { return rgb(245, 99, 72); }
static inline Sizei em_size() { return (Sizei){3, 3}; }

bool em_dead(Entertainment *em) {
  (void)em;
  return false;
}

float em_render_order(Entertainment *em) { return l_to_y(em->display.location.y); }

void em_update(Entertainment *em, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)dt;

  if (em->some_one_is_done) {
    em->some_one_is_done = false;
    bd_flash(&em->display);
  }
  bd_update(&em->display, g);
}

void em_draw(Entertainment *em, GameScene *gs, Game *g) {
  if (ri_contains(em->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Entertainment (%d,%d,%d,%d)\n", em->display.location.x, em->display.location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&em->display, g, em_color(), MI_Entertainment);

  Vec2 p = l_to_vecP(ri_bottom_right(em->display.location));
  const Sizei s = em_size();
  int c = 0;
  for (int j = s.h - 2; j >= 0; --j) {
    for (int i = 0; i < s.w - 1; ++i) {
      float r = 0.0f;
      float s = 0.75f;
      int img = 14;
      if (c < em->started) {
        r += 0.05f * sin(26.0 * g_time(g) + i * j);
        s += 0.01f * sin(17.0 * g_time(g) + i * j);
        working_color(g);
      } else if (c < em->claimed)
        work_claimed_color(g);
      else {
        g_color(g, rgb(255, 255, 255));
        img = 15;
      }
      g_objectRS(g, g_animation_buffer(g), Img_wearisome, img, v_add(p, l_to_vec(i + 1, j + 1)), r, s);
      ++c;
    }
  }
}

static SceneObjectTable Entertainment_table = {
    .dead = (SceneObjectDeadCB)em_dead,
    .render_order = (SceneObjectRenderOrderCB)em_render_order,
    .draw = (SceneObjectDrawCB)em_draw,
    .update = (SceneObjectUpdateCB)em_update,
};

Recti em_location(const Entertainment *em) { return em->display.location; }

bool em_provides(Entertainment *em, GameScene *gs, Resource r) {
  (void)gs;
  const Sizei s = em_size();
  return r == R_Entertainment && em->claimed < (s.w - 1) * (s.h - 1);
}

bool em_done_entainment(void *context, Wearisome *w, GameScene *gs) {
  (void)gs;
  (void)w;
  Entertainment *em = (Entertainment *)context;
  em->started--;
  em->claimed--;
  w->need_mode = W_Normal;
  return false;
}
bool em_start_entainment(void *context, Wearisome *w, GameScene *gs) {
  Entertainment *em = (Entertainment *)context;
  em->started++;
  w_earn_clicks(w, -1);
  gs->clicks++;
  w->need_mode = W_GetEntertainment;
  return w_queue_wait_for(w, 15.0, (QueueItem){em, em_done_entainment});
}
void em_claim(Entertainment *em, GameScene *gs, Wearisome *w, Resource r) {
  assert(r == R_Entertainment);
  if (w_queue_move_to(w, gs, em->display.location, (QueueItem){em, em_start_entainment}))
    em->claimed++;
}

static TileContentTable Entertainment_TileContent_Table = {
    .location = (LocationCb)em_location,
    .provides = (ProvidesCB)em_provides,
    .claim = (ClaimCB)em_claim,
};
Entertainment *Entertainment_init(Game *g, GameScene *gs, Point p) {
  const Sizei s = em_size();
  Entertainment *em = g_malloc(sizeof(Entertainment));
  *em = (Entertainment){
      .display = bd_create(g, (Recti){p.x, p.y, s.w, s.w}),
      .started = 0,
      .claimed = 0,
      .some_one_is_done = false,
  };

  l_set_tile_contentR(gs->level, em->display.location, to_TileContent(em, &Entertainment_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = em, &Entertainment_table});
  return em;
}

#endif