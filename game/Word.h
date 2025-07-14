#ifndef WORD_H
#define WORD_H

#include "game/DataBase.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/effects/Bling.h"
#include "math/Color.h"
#include "math/Vec2.h"
#include "math/random.h"
#include <stdio.h>

typedef struct Word Word;
typedef void (*OnEnteredDone)(Word *, GameScene *gs, Game *g, void *);

typedef struct Word {
  Vec2 pos;
  float alpha;
  char text[32];

  G_Object word_start;
  G_Object word_end;
  int char_reached;
  float word_end_offset;

  bool dead;

  OnEnteredDone word_entered;
  void *word_entered_user_data;

} Word;

bool w_dead(Word *w) { return w->dead; }
float w_render_order(const Word *w) { return w->pos.y + 200; }

static inline void w_destroy(Word *w) {
  G_Object_free(&w->word_start);
  G_Object_free(&w->word_end);
  w->dead = true;
}

void w_set_word(Word *w, const char *text) {
  strncpy(w->text, text, 32);
  w->char_reached = 0;
}
void w_new_word(Word *w, int level) {
  memset(w->text, 0, sizeof(w->text));
  const size_t max = i_min(sizeof(words) / sizeof(words[0]), level_map[level + 1].offset);
  const size_t x = rand() % max;
  w_set_word(w, words[x]);
}

void w_update(Word *w, GameScene *gs, Game *g, float dt) {
  if (w->dead)
    return;

  const size_t l = sizeof(gs->entered_until_now);
  assert(l == 32);
  if (w->char_reached == (int)strlen(w->text)) {
    w->alpha += 3.0f * dt;
    if (w->alpha >= 1.95) {
      if (w->word_entered)
        w->word_entered(w, gs, g, w->word_entered_user_data);
      else
        w_new_word(w, gs->level);
    }
    if (w->alpha < 1.0) {
      float x = w->word_end_offset * w->alpha + r_float() * w->alpha;
      Bling_init(gs, (Vec2){w->pos.x + x, w->pos.y + 1 + r_float() * 3.0f}, c_mix(red(), green(), w->alpha));
    }
    return;
  } else if (w->alpha > 0.001)
    w->alpha *= 0.95f;

  size_t x = 0;
  for (size_t i = 1; i < l; ++i)
    if (strncmp(&gs->entered_until_now[l - i], w->text, i) == 0)
      x = i;

  if (x == strlen(w->text)) {
    gs_add_points(gs, x);
    gs->energy = f_min(32.0, gs->energy + x);
    memset(gs->entered_until_now, 0, l);
  }

  if (!G_Object_valid(&w->word_end) || (int)x != w->char_reached) {
    w->char_reached = (int)x;
    w->word_end_offset = g_create_text(g, &w->word_start, Oswald_Regular_12, str("%.*s", x, w->text)).x;
    g_create_text(g, &w->word_end, Oswald_Regular_12, &w->text[x]);
  }
}

void w_draw(Word *w, GameScene *gs, Game *g) {
  (void)gs;
  if (w->dead)
    return;

  const float a = 1.0f - (w->alpha * w->alpha) / 2.0f;
  if (G_Object_valid(&w->word_start)) {
    d_color(g, alphaf(rgb(88, 136, 22), a));
    d_text(g, w->word_start, Oswald_Regular_12, w->pos);
  }
  if (G_Object_valid(&w->word_end)) {
    d_color(g, alphaf(rgb(168, 159, 128), a));
    d_text(g, w->word_end, Oswald_Regular_12, (Vec2){w->pos.x + w->word_end_offset, w->pos.y});
  }
}

SceneObjectTable Word_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)w_dead,
    .render_order = (SceneObjectRenderOrderCB)w_render_order,
    .update = (SceneObjectUpdateCB)w_update,
    .draw = (SceneObjectDrawCB)w_draw,
};

Word *Word_init(GameScene *gs, Game *g, const char *text) {
  (void)g;

  Word *w = (Word *)g_malloc(sizeof(Word));
  *w = (Word){.alpha = 1.9f, .dead = false};
  if (!text)
    w_new_word(w, 1);
  else
    w_set_word(w, text);

  gs_add_object(gs, (SceneObject){w, &Word_SceneObject_Table});

  return w;
}

#endif