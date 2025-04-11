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

typedef struct Word {
  Vec2 pos;
  float vel;
  char text[32];

  G_Object word_start;
  G_Object word_end;
  int char_reached;
  float word_end_offset;

} Word;

bool w_dead(Word *w) {
  (void)w;
  return false;
}
float w_render_order(const Word *w) { return w->pos.y + 200; }

void w_new_word(Word *w) {
  memset(w->text, 0, sizeof(w->text));
  const size_t x = rand() % (sizeof(words) / sizeof(words[0]));
  strncpy(w->text, words[x], 32);
  w->char_reached = 0;
}

void w_update(Word *w, GameScene *gs, Game *g, float dt) {

  const size_t l = sizeof(gs->entered_until_now);
  assert(l == 32);
  if (w->char_reached == (int)strlen(w->text)) {
    w->vel += 3.0f * dt;
    if (w->vel >= 1.95)
      w_new_word(w);
    if (w->vel < 1.0) {
      float x = w->word_end_offset * w->vel + r_float() * w->vel;
      Bling_init(gs, (Vec2){w->pos.x + x, w->pos.y + 1 + r_float() * 3.0f}, c_mix(red(), green(), w->vel));
    }
    return;
  } else if (w->vel > 0.001)
    w->vel *= 0.95f;

  size_t x = 0;
  for (size_t i = 1; i < l; ++i)
    if (strncmp(&gs->entered_until_now[l - i], w->text, i) == 0)
      x = i;

  if (x == strlen(w->text)) {
    gs->points += x;
    gs->points_flush = 1.0f;
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

  const float a = 1.0f - (w->vel * w->vel) / 2.0f;
  if (G_Object_valid(&w->word_start)) {
    g_color(g, alphaf(rgb(88, 136, 22), a));
    g_text(g, w->word_start, Oswald_Regular_12, w->pos);
  }
  if (G_Object_valid(&w->word_end)) {
    g_color(g, alphaf(rgb(168, 159, 128), a));
    g_text(g, w->word_end, Oswald_Regular_12, (Vec2){w->pos.x + w->word_end_offset, w->pos.y});
  }
}

SceneObjectTable Word_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)w_dead,
    .render_order = (SceneObjectRenderOrderCB)w_render_order,
    .update = (SceneObjectUpdateCB)w_update,
    .draw = (SceneObjectDrawCB)w_draw,
};

Word *Word_init(GameScene *gs, Game *g) {
  (void)g;

  Word *w = (Word *)g_malloc(sizeof(Word));
  *w = (Word){.vel = 1.9f};
  w_new_word(w);

  gs_add_object(gs, (SceneObject){w, &Word_SceneObject_Table});

  return w;
}

#endif