#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/MoveMarker.h"
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
  Vec2 destination;
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

  w->frame += (dt * 8.0f);
  w->position = v_lerp_about(w->position, w->destination, dt * 24.0f);
}

void Wearisome_draw(Wearisome *w, Game *g) {
  const float time = Game_time(g);

  Vec2 p = v_add(w->position, (Vec2){0, 5});

  d_noise(g, 0.0f);
  d_color(g, alphaf(w->color, w->highlight ? 0.9f : (w->active ? 0.8f : 0.4f)));
  d_object(g, d_animation_buffer(g), w->marker, p, (int)w->frame % 4);

  d_noise(g, w->highlight ? 0.1 : 0.01f);
  d_color(g, (Color){0.01f * sin(time * 20) + 0.9f, 1.0f, 1.0f, 1.0f});
  if (v_eq(w->position, w->destination))
    d_object(g, d_animation_buffer(g), w->texture, p, 4 + (int)w->frame % 8);
  else
    d_object(g, d_animation_buffer(g), w->texture, p, (int)w->frame % 4);
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

static bool Wearisome_still_activate(Wearisome *w) { return w->active && !Wearisome_dead(w); }
static void Wearisome_move(Wearisome *w, Vec2 p) { w->destination = p; }
static void Wearisome_activate(Wearisome *w, Game *g, GameScene *gs) {
  (void)g;
  (void)gs;
  w->active = true;

  int mi = (int)(w->position.x / 16.0f);
  int mj = (int)(w->position.y / 16.0f);

  for (int i = mi - 3; i <= mi + 3; ++i)
    for (int j = mj - 3; j <= mj + 3; ++j) {
      if ((i == mi && j == mj) || !map_is_set(i, j))
        continue;
      MoveMarker_init(g, gs, (Vec2){i * 16.0f, j * 16.0f}, w, (MoveMarkerContextActiveCB)Wearisome_still_activate,
                      (MoveMarkerContextMoveCB)Wearisome_move);
    }
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
      .destination = pos,
      .color = col,
      .frame = rand() % 52,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

#endif