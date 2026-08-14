#ifndef WORDBUBBLE_H
#define WORDBUBBLE_H

#include "engine/Transformation.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/SpaceShip.h"
#include "game/Word.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Vec2.h"

typedef struct WordBubble {
  LineObject lines;
  Vec2 pos, connect, offset;
  Word *word;
} WordBubble;

bool wb_dead(const WordBubble *wb) { return wb->word == NULL; }
float wb_render_order(const WordBubble *wb) { return wb->pos.y + 100; }

void wb_update(WordBubble *wb, GameScene *gs, Game *g, float dt) {
  (void)g;

  if (!wb->word)
    return;
  if (gs->health <= 0.0) {
    w_destroy(wb->word);
    wb->word = NULL;
    return;
  }

  Vec2 b = v_add(gs->spaceship->pos, wb->offset);
  wb->pos = v_lerp(wb->pos, b, 0.25f * dt);
  wb->word->pos = gs->health <= 0 ? (Vec2){-1000, -1000} : v_add(wb->pos, (Vec2){-45, -3});

  Vec2 a = gs->spaceship->pos;
  b = v_add(wb->pos, (Vec2){48.0, 10.0});
  Vec2 coords[10];
  for (int i = 0; i < 10; ++i) {
    float t = (float)i / 9.0f;
    t *= t;
    coords[i].y = a.y + t * (b.y - a.y);
    t *= t;
    coords[i].x = a.x + t * (b.x - a.x);
  }

  g_update_line_strip(g, &wb->lines, coords, 10);
}

void wb_draw(WordBubble *wb, GameScene *gs, Game *g) {
  if (gs->health <= 0.0)
    return;

  d_color(g, alphaf(white(), gs->initialized));

  Vec2 a = v_add(gs->spaceship->pos, wb->connect);
  Vec2 b = wb->pos;
  Vec2 p = a;
  for (int i = 0; i < 10; ++i) {
    float t = (float)i / 9.0f;
    t *= t;
    p.y = a.y + t * (b.y - a.y);
    t *= t;
    p.x = a.x + t * (b.x - a.x);
    // p = v_lerp(a, b, t);
    d_animation(g, Img_starship, 4, t_PS(p, vec2f(0.25f + t * 7.0)));
  }

  d_color(g, red());
  d_lines(g, &wb->lines, t_P((Vec2){0, -8.0f}));
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

  g_create_line_strip(g, &wb->lines, 10);

  return wb;
}

#endif