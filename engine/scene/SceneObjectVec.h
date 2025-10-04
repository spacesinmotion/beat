#ifndef SCENEOBJECTVEC_H
#define SCENEOBJECTVEC_H

#include "engine/scene/SceneObject.h"

typedef struct Game Game;

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

SceneObjectVec so_vec_empty();

void so_vec_push(SceneObjectVec *vec, Game *g, SceneObject so);

void so_vec_update_all(SceneObjectVec *vec, Game *g);
void so_vec_draw_all(SceneObjectVec *vec, Game *g);

void so_vec_filter_dead(SceneObjectVec *vec, Game *g);
void so_vec_sort_by_render_order(SceneObjectVec *vec);

#ifdef GAME_ENGINE_IMPL

SceneObjectVec so_vec_empty() { return (SceneObjectVec){NULL, 0, 0}; }

void *g_realloc(Game *g, void *ptr, size_t size);
void so_vec_push(SceneObjectVec *vec, Game *g, SceneObject so) {
  if (vec->len + 1 > vec->cap) {
    vec->cap += 16;
    vec->data = (SceneObject *)g_realloc(g, vec->data, vec->cap * sizeof(SceneObject));
  }
  vec->data[vec->len] = so;
  vec->len++;
}

void so_vec_filter_dead(SceneObjectVec *vec, Game *g) {
  for (int i = vec->len - 1; i >= 0; --i) {
    if (!vec->data[i].table->die(vec->data[i].context, g))
      continue;
    vec->data[i] = vec->data[vec->len - 1];
    vec->data[vec->len - 1] = (SceneObject){NULL, NULL};
    --vec->len;
  }
}

void so_vec_update_all(SceneObjectVec *vec, Game *g) {
  for (int i = 0; i < vec->len; ++i)
    so_update(&vec->data[i], g);
}

void so_vec_draw_all(SceneObjectVec *vec, Game *g) {
  for (int i = 0; i < vec->len; ++i)
    so_draw(&vec->data[i], g);
}

int so_render_order_compare(const void *va, const void *vb) {
  const float a = so_render_order((SceneObject *)va);
  const float b = so_render_order((SceneObject *)vb);
  return a < b ? -1 : (a > b ? 1 : 0);
}

void so_vec_sort_by_render_order(SceneObjectVec *vec) {
  qsort(vec->data, vec->len, sizeof(SceneObject), so_render_order_compare);
}

#endif

#endif