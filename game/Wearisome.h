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
#include <stdlib.h>

typedef struct Wearisome {
  const sg_image *texture;
  const sg_image *marker;

  Color color;
  Vec2 position;
  float frame;
  bool highlight;
  bool active;
} Wearisome;

bool Wearisome_dead(Wearisome *w) {
  (void)w;
  return false;
}

void Wearisome_update(Wearisome *w, Game *g, float dt) {
  (void)g;

  w->frame += (dt * 8);
}

void Wearisome_draw(Wearisome *w, Game *g) {
  const float time = Game_time(g);

  d_noise(g, 0.0f);
  d_color(g, alphaf(w->color, w->highlight ? 0.9f : (w->active ? 0.8f : 0.4f)));
  d_object(g, d_animation_buffer(g), w->marker, w->position, (int)w->frame % 4);

  d_noise(g, w->highlight ? 0.3 : 0.1f);
  d_color(g, (Color){0.01f * sin(time * 20) + 0.9f, 1.0f, 1.0f, 1.0f});
  d_object(g, d_animation_buffer(g), w->texture, w->position, ((int)w->frame % 16 > 14) ? 1 : 0);
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

static void Wearisome_activate(Wearisome *w, Game *g, GameScene *gs) {
  (void)g;
  (void)gs;
  w->active = true;
}

static void Wearisome_deactivate(Wearisome *w, Game *g, GameScene *gs) {
  (void)g;
  (void)gs;
  w->active = false;
}

SceneObjectTable Wearisome_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)Wearisome_dead,
    .update = (SceneObjectUpdateCB)Wearisome_update,
    .draw = (SceneObjectDrawCB)Wearisome_draw,
    .mouse_hit = (SceneObjectMouseHitCB)Wearisome_mouse_hit,
    .enter = (SceneObjectEnterCB)Wearisome_enter,
    .leave = (SceneObjectLeaveCB)Wearisome_leave,
    .activate = (SceneObjectActivateCB)Wearisome_activate,
    .deactivate = (SceneObjectDeactivateCB)Wearisome_deactivate,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, Vec2 pos, Color col) {
  Wearisome *w = gc_malloc(&gc, sizeof(Wearisome));
  *w = (Wearisome){
      .texture = g_image(g, Img_wearisome),
      .marker = g_image(g, Img_marker),
      .position = pos,
      .color = col,
      .frame = rand(),
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif