#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Rect.h"
#include "math/Vec2.h"

#include <math.h>

typedef struct Wearisome {
  const sg_image *texture;

  Vec2 position;
  int frame;
  bool highlight;
} Wearisome;

bool Wearisome_dead(Wearisome *w) {
  (void)w;
  return false;
}

void Wearisome_update(Wearisome *w, Game *g, float dt) {
  (void)dt;

  const float time = Game_time(g);
  const int frame = (int)(time * 4);
  if (frame % 32 > 29)
    w->frame = 1;
  else
    w->frame = 0;
}
void Wearisome_draw(Wearisome *w, Game *g) {
  const float time = Game_time(g);
  d_noise(g, w->highlight ? 0.9 : 0.1f);
  d_color(g, 0.01f * sin(time * 20) + 0.9f, 1.0f, 1.0f, 1.0f);
  d_object(g, d_animation_buffer(g), w->texture, w->position, w->frame);
}

static bool Wearisome_mouse_hit(Wearisome *w, Vec2 mp) {
  return Rect_contains((Rect){w->position, (Vec2){16, 16}}, mp);
}

static void Wearisome_enter(Wearisome *w, Game *g) {
  (void)g;
  w->highlight = true;
}

static void Wearisome_leave(Wearisome *w, Game *g) {
  (void)g;
  w->highlight = false;
}

static void Wearisome_click(Wearisome *w, Game *g, int button) {
  (void)g;
  (void)button;
  w->highlight = !w->highlight;
}

SceneObjectTable Wearisome_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Wearisome_dead,
    .update = (SceneObjectUpdateCB)Wearisome_update,
    .draw = (SceneObjectDrawCB)Wearisome_draw,
    .mouse_hit = (SceneObjectMouseHitCB)Wearisome_mouse_hit,
    .enter = (SceneObjectEnterCB)Wearisome_enter,
    .leave = (SceneObjectLeaveCB)Wearisome_leave,
    .click = (SceneObjectClickCB)Wearisome_click,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, Vec2 pos) {
  Wearisome *w = gc_malloc(&gc, sizeof(Wearisome));
  *w = (Wearisome){
      .texture = g_image(g, Img_wearisome),
      .position = pos,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif