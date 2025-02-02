#ifndef WORKPROVIDER_H
#define WORKPROVIDER_H

#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "math/Color.h"
#include "math/Vec2.h"
#include <assert.h>

typedef struct WorkProvider {
  int colums, rows;
  int clicks, clicks_claimed, clicks_work, clicks_done;
  int storage;
  float work_duration;
} WorkProvider;

static inline void wp_init(WorkProvider *wp, int c, int r, float work_duration) {
  *wp = (WorkProvider){c, r, 0, 0, 0, 0, 0, work_duration};
}
static inline void wp_reduce_clicks(WorkProvider *wp, int count) {
  wp->clicks -= count;
  wp->clicks_claimed -= count;
  wp->clicks_work -= count;
  wp->clicks_done -= count;
}
static inline int wp_fields(const WorkProvider *wp) { return wp->colums * wp->rows; }
static inline bool wp_is_done(const WorkProvider *wp) { return wp->clicks_done == wp_fields(wp); }

bool wp_provides(WorkProvider *wp, GameScene *gs, Resource r) {
  (void)gs;
  return r == R_Work && wp->clicks - wp->clicks_claimed > 0;
}
void wp_claim(WorkProvider *wp, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  wp->clicks_claimed++;
}
float wp_start(WorkProvider *wp, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  wp->clicks_work++;
  return wp->work_duration;
}
void wp_done(WorkProvider *wp, GameScene *gs, Resource r) {
  (void)gs;
  assert(r == R_Work);
  wp->clicks_done++;
}
void wp_click(WorkProvider *wp, GameScene *gs) {
  (void)gs;
  if (wp->clicks < wp_fields(wp) && gs->clicks > 0) {
    wp->clicks++;
    gs->clicks--;
  }
}

static inline void wp_draw_click_fields(const WorkProvider *wp, Game *g, Vec2 p, bool ignore_empty) {
  int c = 0;
  for (int j = wp->rows - 1; j >= 0; --j) {
    for (int i = 0; i < wp->colums; ++i) {
      int img = 12;
      if (c < wp->clicks_done)
        done_color(g);
      else if (c < wp->clicks_work)
        working_color(g);
      else if (c < wp->clicks_claimed)
        work_claimed_color(g);
      else if (c < wp->clicks)
        clicked_color(g);
      else if (ignore_empty)
        return;
      else {
        g_color(g, rgb(255, 255, 255));
        img = 13;
      }
      g_objectS(g, g_animation_buffer(g), Img_wearisome, img, v_add(p, l_to_vec(i, j)), 0.75f);
      ++c;
    }
  }
}

static inline bool wp_finish_production_cycle(WorkProvider *wp, int max_storage) {
  bool need_flash = false;
  for (int i = wp->storage; i < max_storage && wp->clicks_done > 0; ++i) {
    wp->storage++;
    wp_reduce_clicks(wp, 1);
    need_flash = true;
  }
  return need_flash;
}

static inline bool wp_has_something_stored(const WorkProvider *wp) { return wp->storage > 0; }

static TileContentTable WorkProvider_TileContent_Default_Table = {
    .provides = (ProvidesCB)wp_provides,
    .claim = (ClaimCB)wp_claim,
    .start = (StartCB)wp_start,
    .done = (DoneCB)wp_done,
    .click = (ClickCB)wp_click,
};

#endif