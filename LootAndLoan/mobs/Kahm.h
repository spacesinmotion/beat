#ifndef KAHM_H
#define KAHM_H

#include "LootAndLoan/DungeonScene.h"
#include "engine/Game.h"
#include "engine/math/Vec2.h"
#include "engine/scene/SceneObject.h"

typedef struct Kahm {
  DrawEntity *emo;

  Vec2 position, destination;
  float health;

  bool turn_finished;
} Kahm;

static bool kh_die(Kahm *kh, Game *g, bool force) {
  if (force || kh->health <= 0.0) {
    de_free(g, kh->emo);
    g_free(g, kh);
    return true;
  }
  return false;
}

static float kh_render_order(const Kahm *kh) { return kh->position.y; }

static void kh_update(Kahm *kh, Game *g) {
  (void)kh, (void)g;
  if (!v_eq(kh->position, kh->destination)) {
    kh->position = v_lerp_about(kh->position, kh->destination, 16 * 2.0f * g_animation_delta(g));
    kh->turn_finished = v_eq(kh->position, kh->destination);
  }
}

static void kh_draw(const Kahm *kh, Game *g) {
  const Vec2 p = kh->position;
  g_color(g, green());
  Vec2 s = {1.0 + 0.025 * sin(4 * g_time(g)), 1.0 - 0.015 * sin(4 * g_time(g))};
  g_draw_entity(g, kh->emo, Img_emo, dt_prs(p, 0.02 * sin(g_time(g)), s));
}

static void kh_save(CJHObject *o, const Kahm *kh) {
  (void)o;
  (void)kh;
  // TODO: Implement save logic
}

void kh_start_turn(Kahm *kh, Game *g, Point d) {

  ds_set_map(ds_get(g), ds_to_grid(kh->destination), MT_Empty);

  const Point p = ds_to_grid(kh->position);
  Point dest = ds_map_step_to_player(g, p, 9);
  if (dest.x >= 0 && dest.y >= 0)
    kh->destination = ds_from_grid(dest);

  ds_set_map(ds_get(g), ds_to_grid(kh->destination), MT_Mob);
  kh->turn_finished = v_eq(kh->position, kh->destination);
}

static SceneObjectTable Kahm_table = {
    .type = "Kahm",
    .die = (SceneObjectDieCB)kh_die,
    .render_order = (SceneObjectRenderOrderCB)kh_render_order,
    .update = (SceneObjectUpdateCB)kh_update,
    .draw = (SceneObjectDrawCB)kh_draw,
    .save = (SceneObjectSaveCB)kh_save,
};

Kahm *Kahm_create(DungeonScene *ds, Game *g, Vec2 pos) {
  Kahm *kh = g_malloc(g, sizeof(Kahm));

  *kh = (Kahm){
      .emo = g_sub_image(g, (Sizei){512, 512}, (Point){23, 25}, (Point[4]){{14, 4}, {33, 4}, {33, 31}, {14, 31}}),
      .position = pos,
      .destination = pos,
      .health = 100.0f,
      .turn_finished = true,
  };

  ds_add_object(ds, g, (SceneObject){kh, &Kahm_table});
  ds_set_map(ds, ds_to_grid(pos), MT_Mob);

  return kh;
}

#endif // KIRC_H