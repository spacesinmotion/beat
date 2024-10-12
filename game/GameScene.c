
#include "game/SceneObject.h"
#include "gc/gc.h"

#include "Wearisome.h"

typedef struct SceneObjectVec {
  SceneObject *data;
  int len, cap;
} SceneObjectVec;

void SceneObjectVec_push(SceneObjectVec *vec, SceneObject so) {
  if (vec->len + 1 > vec->cap) {
    vec->cap += 16;
    vec->data = (SceneObject *)gc_realloc(&gc, vec->data, vec->cap * sizeof(SceneObject));
  }
  vec->data[vec->len] = so;
  vec->len++;
}

void SceneObjectVec_filter_dead(SceneObjectVec *vec) {
  for (int i = vec->len - 1; i > 0; --i) {
    if (!vec->data[i].table->dead(vec->data[i].context))
      continue;
    vec->data[i] = vec->data[vec->len - 1];
    --vec->len;
  }
}

typedef struct GameScene {
  SceneObjectVec scene_objects;
} GameScene;

void GameScene_draw(Game *g, GameScene *scene) {
  (void)scene;

  d_noise(g, 0.01f);
  d_buffer(g, d_tilemap_buffer(g), d_tilemap_image(g), (Vec2){-8, 8});

  for (int i = 0; i < scene->scene_objects.len; ++i)
    SceneObject_update(&scene->scene_objects.data[i], g);
  SceneObjectVec_filter_dead(&scene->scene_objects);
  for (int i = 0; i < scene->scene_objects.len; ++i)
    SceneObject_draw(&scene->scene_objects.data[i], g);
}

void GameScene_add_object(GameScene *gs, SceneObject so) { SceneObjectVec_push(&gs->scene_objects, so); }

void GameScene_init(Game *g) {
  GameScene *gs = gc_malloc(&gc, sizeof(GameScene));
  *gs = (GameScene){0};

  Wearisome_init(g, gs);

  game_set_scene(g, (SceneDrawCB)GameScene_draw, gs);
}