#ifndef WORKPROVIDER_H
#define WORKPROVIDER_H

#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/Wearisome.h"

typedef struct WorkProvider {
  int colums, rows;
  int clicks, clicks_claimed, clicks_work, clicks_done;
  int storage, storage_claimed;
} WorkProvider;

static inline void wp_init(WorkProvider *wp, int c, int r) { *wp = (WorkProvider){c, r, 0, 0, 0, 0, 0, 0}; }
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
static inline bool wp_has_work(WorkProvider *wp, GameScene *gs) { return gs->clicks > 0 && wp->clicks < wp_fields(wp); }
void wp_click(WorkProvider *wp, GameScene *gs) {
  (void)gs;
  if (wp_has_work(wp, gs)) {
    wp->clicks++;
    gs->clicks--;
  }
}

void wp_claim(WorkProvider *wp) { wp->clicks_claimed++; }
void wp_start(WorkProvider *wp, Wearisome *w) {
  wp->clicks_work++;
  w->need_mode = W_IsWorking;
}
void wp_done(WorkProvider *wp, Wearisome *w) {
  wp->clicks_done++;
  w_earn_clicks(w, 1);
  w->need_mode = W_Normal;
}

static inline void wp_draw_click_fields(const WorkProvider *wp, Game *g, Vec2 p, bool ignore_empty) {
  int c = 0;
  for (int j = wp->rows - 1; j >= 0; --j) {
    for (int i = 0; i < wp->colums; ++i) {
      float r = 0.0f;
      float s = 0.75f;
      int img = 12;
      if (c < wp->clicks_done)
        done_color(g);
      else if (c < wp->clicks_work) {
        working_color(g);
        r += 0.05f * sin(26.0 * g_time(g) + i * j);
        s += 0.01f * sin(17.0 * g_time(g) + i * j);
      } else if (c < wp->clicks_claimed)
        work_claimed_color(g);
      else if (c < wp->clicks)
        clicked_color(g);
      else if (ignore_empty)
        return;
      else {
        g_color(g, rgb(255, 255, 255));
        img = 13;
      }
      g_objectRS(g, g_animation_buffer(g), Img_wearisome, img, v_add(p, l_to_vec(i, j)), r, s);
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
static inline bool wp_has_something_to_deliver(const WorkProvider *wp) { return wp->storage - wp->storage_claimed > 0; }

static inline void wp_claim_storage(WorkProvider *wp, GameScene *gs, Wearisome *w) {
  const int count = (wp->storage - wp->storage_claimed) > 1 ? 2 : 1;
  wp->storage_claimed += count;
  gs->storage_claimed += count;
  w_deliver_claim(w, count);
}
static inline bool wp_storage_taken(WorkProvider *wp, Wearisome *w) {
  wp->storage_claimed -= w->deliver_count;
  wp->storage -= w->deliver_count;
  return true;
}

#endif