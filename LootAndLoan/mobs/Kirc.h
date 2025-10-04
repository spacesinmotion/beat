#ifndef KIRC_H
#define KIRC_H

#include "engine/Game.h"
#include "engine/scene/SceneObject.h"

typedef struct DungeonScene DungeonScene;
void ds_add_object(DungeonScene *ds, Game *g, SceneObject so);

typedef struct Kirc {
  DrawEntity *emo;

  Vec2 position;
  float health;
} Kirc;

static bool kc_die(Kirc *kc, Game *g, bool force) {
  if (force || kc->health <= 0.0) {
    de_free(g, kc->emo);
    g_free(g, kc);
    return true;
  }
  return false;
}

static float kc_render_order(const Kirc *kc) { return kc->position.y; }

static void kc_update(Kirc *kc, Game *g) { kc->position.x += 0.1f * sin(g_time(g)); }

static void kc_draw(const Kirc *kc, Game *g) {
  const Vec2 p = kc->position;
  g_color(g, white());
  Vec2 s = {1.0 + 0.025 * sin(4 * g_time(g)), 1.0 - 0.015 * sin(4 * g_time(g))};
  g_draw_entity(g, kc->emo, Img_emo, dt_prs(p, 0.02 * sin(g_time(g)), s));
}

static void kc_save(CJHObject *o, const Kirc *kc) {
  (void)o;
  (void)kc;
  // TODO: Implement save logic
}

static SceneObjectTable Kirc_table = {
    .type = "Kirc",
    .die = (SceneObjectDieCB)kc_die,
    .render_order = (SceneObjectRenderOrderCB)kc_render_order,
    .update = (SceneObjectUpdateCB)kc_update,
    .draw = (SceneObjectDrawCB)kc_draw,
    .save = (SceneObjectSaveCB)kc_save,
};

Kirc *Kirc_create(DungeonScene *ds, Game *g, Vec2 pos) {
  Kirc *kc = g_malloc(g, sizeof(Kirc));

  *kc = (Kirc){
      .emo = g_sub_image(g, (Sizei){512, 512}, (Point){23, 25}, (Point[4]){{14, 4}, {33, 4}, {33, 31}, {14, 31}}),
      .position = pos,
      .health = 100.0f,
  };

  ds_add_object(ds, g, (SceneObject){kc, &Kirc_table});

  return kc;
}

#endif // KIRC_H