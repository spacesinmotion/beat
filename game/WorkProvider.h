#ifndef WORKPROVIDER_H
#define WORKPROVIDER_H

#include "game/GameColors.h"
#include "game/GameScene.h"
#include "game/Wearisome.h"
#include "math/Color.h"

typedef struct WorkProvider {
  int colums, rows;
  int clicks, clicks_claimed, clicks_work, clicks_done;
  int local_storage, local_storage_claimed;
} WorkProvider;

static inline void wp_init(WorkProvider *wp, int c, int r) { *wp = (WorkProvider){c, r, 0, 0, 0, 0, 0, 0}; }
static inline void wp_reduce_clicks(WorkProvider *wp, int count) {
  wp->clicks -= count;
  wp->clicks_claimed -= count;
  wp->clicks_work -= count;
  wp->clicks_done -= count;
}
static inline void wp_clear_done_work(WorkProvider *wp) { wp_reduce_clicks(wp, wp->clicks_done); }
static inline int wp_fields(const WorkProvider *wp) { return wp->colums * wp->rows; }
static inline bool wp_is_done(const WorkProvider *wp) { return wp->clicks_done == wp_fields(wp); }

int wp_max_storage(const WorkProvider *wp) { return 4 * wp->rows; }
int wp_storage_taken(const WorkProvider *wp) { return wp->local_storage + wp->clicks - wp->clicks_done; }

bool wp_provides(WorkProvider *wp, GameScene *gs, Resource r) {
  (void)gs;
  return r == R_Work && wp->clicks - wp->clicks_claimed > 0;
}
static inline bool wp_has_work(WorkProvider *wp) { return wp->clicks < wp_fields(wp); }
void wp_click(WorkProvider *wp, Point p, GameScene *gs) {
  (void)p;
  if (gs->clicks > 0 && wp_has_work(wp)) {
    wp->clicks++;
    gs->clicks--;
  }
}
void wp_click_with_storage(WorkProvider *wp, Point p, GameScene *gs) {
  (void)p;
  if (gs->clicks > 0 && wp_has_work(wp) && wp_storage_taken(wp) < wp_max_storage(wp)) {
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
void wp_done_and_store(WorkProvider *wp, Wearisome *w) {
  wp_done(wp, w);
  wp->local_storage++;
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
        s += 0.02f * sin(17.0 * g_time(g) + i * j);
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

static inline void wp_draw_storage(const WorkProvider *wp, Game *g, Vec2 p) {
  const int ms = wp_max_storage(wp);
  const float h = wp->local_storage >= ms ? 1.0f : (wp->local_storage > ms - 4 ? 0.4f : 0.0f);
  g_color(g, wp->local_storage >= ms ? critical_color() : (wp->local_storage > ms - 4 ? warn_color() : white()));
  for (int i = 0; i < wp->local_storage; i += 4)
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 8 + i_min(wp->local_storage - i, 4) - 1,
              v_add(p, l_to_vec(0, i / 4)), 0.75 + h * 0.02f * sin(17.0 * g_time(g) + i));
}

static inline bool wp_has_something_to_deliver(const WorkProvider *wp) {
  return wp->local_storage - wp->local_storage_claimed > 0;
}

static inline void wp_claim_deliver(WorkProvider *wp, GameScene *gs) {
  wp->local_storage_claimed++;
  gs->storage_claimed++;
}
static inline bool wp_deliver_taken(WorkProvider *wp) {
  wp->local_storage_claimed--;
  wp->local_storage--;
  // wp_reduce_clicks(wp, 1);
  return true;
}

static inline void wp_to_json(CJHObject *o, WorkProvider *wp) {
  cjh_o_add_number(o, "colums", wp->colums);
  cjh_o_add_number(o, "rows", wp->rows);
  cjh_o_add_number_if(o, "clicks", wp->clicks, 0);
  cjh_o_add_number_if(o, "clicks_claimed", wp->clicks_claimed, 0);
  cjh_o_add_number_if(o, "clicks_work", wp->clicks_work, 0);
  cjh_o_add_number_if(o, "clicks_done", wp->clicks_done, 0);
  cjh_o_add_number_if(o, "local_storage", wp->local_storage, 0);
  cjh_o_add_number_if(o, "local_storage_claimed", wp->local_storage_claimed, 0);
}

static inline void wp_from_json(CJHObjectR *o, const char *key, WorkProvider *wp) {
  (void)wp;

  if (streq(key, "colums"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "rows"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "clicks"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "clicks_claimed"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "clicks_work"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "clicks_done"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "clicks_claimed_for_deliver"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

#endif