#ifndef WORDBUBBLE_H
#define WORDBUBBLE_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/SpaceShip.h"
#include "game/Word.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Vec2.h"

typedef struct WordBubble {
  Vec2 pos, connect, offset;
  Word *words[7];
} WordBubble;

bool wb_dead(const WordBubble *wb) {
  (void)wb;
  return false;
}
float wb_render_order(const WordBubble *wb) { return wb->pos.y + 100; }

void wb_update(WordBubble *wb, GameScene *gs, Game *g, float dt) {
  (void)gs;
  (void)g;
  (void)dt;

  Vec2 b = v_add(gs->spaceship->pos, wb->offset);
  wb->pos = v_lerp(wb->pos, b, 0.25f * dt);
  for (int i = 0; i < 7; ++i)
    wb->words[i]->pos = v_add(wb->pos, (Vec2){-45, -45 + i * 14});
}

void wb_draw(WordBubble *wb, GameScene *gs, Game *g) {
  (void)gs;

  g_color(g, white());

  Vec2 a = v_add(gs->spaceship->pos, wb->connect);
  Vec2 b = wb->pos;
  Vec2 p = a;
  for (int i = 0; i <= 9; ++i) {
    float t = (float)i / 9.0f;
    t *= t;
    p.y = a.y + t * (b.y - a.y);
    t *= t;
    p.x = a.x + t * (b.x - a.x);
    // p = v_lerp(a, b, t);
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, p, 0.25f + t * 7.0);
  }
}

SceneObjectTable WordBubble_SceneObject_Table = {
    .dead = (SceneObjectDeadCB)wb_dead,
    .render_order = (SceneObjectRenderOrderCB)wb_render_order,
    .update = (SceneObjectUpdateCB)wb_update,
    .draw = (SceneObjectDrawCB)wb_draw,
};

WordBubble *WordBubble_init(GameScene *gs, Game *g) {
  WordBubble *wb = (WordBubble *)g_malloc(sizeof(WordBubble));
  *wb = (WordBubble){
      .connect = (Vec2){-3, -8},
      .offset = (Vec2){-85, -70},
  };

  gs_add_object(gs, (SceneObject){wb, &WordBubble_SceneObject_Table});

  for (int i = 0; i < 7; ++i) {
    size_t w = rand() % (sizeof(words) / sizeof(words[0]));
    printf("%d %s\n", (int)w, words[w]);
    wb->words[i] = Word_init(gs, g);
  }

  return wb;
}

#endif