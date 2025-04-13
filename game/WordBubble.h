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
  Word *word;
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
  wb->word->pos = v_add(wb->pos, (Vec2){-45, -3});
}

void wb_draw(WordBubble *wb, GameScene *gs, Game *g) {
  (void)gs;

  g_color(g, alphaf(white(), gs->initialized));

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
    g_objectS(g, g_animation_buffer(g), Img_starship, 4, p, vec2f(0.25f + t * 7.0));
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
      .offset = (Vec2){-65, -25},
  };
  wb->pos = v_add(gs->spaceship->pos, wb->offset);

  gs_add_object(gs, (SceneObject){wb, &WordBubble_SceneObject_Table});

  wb->word = Word_init(gs, g, NULL);

  return wb;
}

#endif