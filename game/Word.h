#ifndef WORD_H
#define WORD_H

#include "game/DataBase.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/effects/Bling.h"
#include "math/Color.h"
#include "math/Vec2.h"

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
  if (w->vel >= 2.0f) {
    G_Object_free(&w->word_end);
    G_Object_free(&w->word_start);
    return true;
  }
  return false;
}
float w_render_order(const Word *w) { return w->pos.y; }

Word *Word_init(GameScene *gs, Game *g, const char *text, Vec2 pos);
void w_update(Word *w, GameScene *gs, Game *g, float dt) {

  const size_t l = sizeof(gs->entered_until_now);
  assert(l == 32);
  if (w->char_reached == (int)strlen(w->text)) {
    float ov = w->vel;
    w->vel += dt;
    w->pos.y -= w->vel;
    if (ov < 0.65 && w->vel >= 0.65) {
      size_t w = rand() % (sizeof(words) / sizeof(words[0]));
      printf("%d %s\n", (int)w, words[w]);
      Word_init(gs, g, words[w], (Vec2){100, 100});
    }
    return;
  } else if (w->vel > 0.001)
    w->vel *= 0.95f;

  size_t x = 0;
  for (size_t i = 1; i < l; ++i)
    if (strncmp(&gs->entered_until_now[l - i], w->text, i) == 0)
      x = i;

  if (x == strlen(w->text)) {
    memset(gs->entered_until_now, 0, l);
    Bling_init(gs, (Vec2){w->pos.x + rand() % 50, w->pos.y + 10}, red());
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
    g_color(g, alphaf(rgb(76, 92, 53), a));
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

Word *Word_init(GameScene *gs, Game *g, const char *text, Vec2 pos) {
  (void)g;

  Word *w = (Word *)g_malloc(sizeof(Word));
  *w = (Word){.pos = pos, .vel = 1.9f};

  memset(w->text, 0, sizeof(w->text));
  strncpy(w->text, text, 32);

  gs_add_object(gs, (SceneObject){w, &Word_SceneObject_Table});

  return w;
}

#endif