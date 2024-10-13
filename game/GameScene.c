
#include "Wearisome.h"
#include "game/Game.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Vec2.h"

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
    vec->data[vec->len - 1] = (SceneObject){NULL, NULL};
    --vec->len;
  }
}

typedef struct GameScene {
  SceneObjectVec scene_objects;
  const sg_image *tilemap_img;

  SceneObject under_mouse;
  SceneObject active;
} GameScene;

void GameScene_update(GameScene *gs, Game *g, float dt) {
  for (int i = 0; i < gs->scene_objects.len; ++i)
    SceneObject_update(&gs->scene_objects.data[i], g, dt);
  SceneObjectVec_filter_dead(&gs->scene_objects);
  if (SceneObject_dead(&gs->active))
    gs->active = (SceneObject){NULL, NULL};
  if (SceneObject_dead(&gs->under_mouse))
    gs->under_mouse = (SceneObject){NULL, NULL};
}

void GameScene_draw(GameScene *gs, Game *g) {
  d_noise(g, 0.0f);
  d_color(g, white());
  d_buffer(g, d_tilemap_buffer(g), gs->tilemap_img, (Vec2){8, 8});

  for (int i = 0; i < gs->scene_objects.len; ++i)
    SceneObject_draw(&gs->scene_objects.data[i], g);
}

void GameScene_mouse_move(GameScene *gs, Game *g, Vec2 mp) {

  SceneObject new_under_mouse = (SceneObject){0};
  for (int i = gs->scene_objects.len - 1; i >= 0; --i)
    if (SceneObject_mouse_hit(&gs->scene_objects.data[i], mp)) {
      new_under_mouse = gs->scene_objects.data[i];
      break;
    }

  if (new_under_mouse.context == gs->under_mouse.context)
    return;

  SceneObject_leave(&gs->under_mouse, g);
  gs->under_mouse = new_under_mouse;
  SceneObject_enter(&gs->under_mouse, g);
}

void GameScene_click(GameScene *gs, Game *g, Vec2 mp, int button) {
  (void)mp;

  if (gs->under_mouse.context != gs->active.context) {
    SceneObject_deactivate(&gs->active, g, gs);
    gs->active = gs->under_mouse;
    SceneObject_activate(&gs->active, g, gs);
  }

  SceneObject_click(&gs->under_mouse, g, gs, button);
}

void GameScene_add_object(GameScene *gs, SceneObject so) { SceneObjectVec_push(&gs->scene_objects, so); }

void GameScene_init(Game *g) {
  GameScene *gs = gc_malloc(&gc, sizeof(GameScene));
  *gs = (GameScene){
      .tilemap_img = g_image(g, Img_tilemap),
  };

  Wearisome_init(g, gs, (Vec2){1 * 16, 0}, rgb(231, 69, 38));
  Wearisome_init(g, gs, (Vec2){2 * 16, 0}, rgb(77, 213, 30));
  Wearisome_init(g, gs, (Vec2){3 * 16, 0}, rgb(35, 87, 200));
  Wearisome_init(g, gs, (Vec2){4 * 16, 0}, rgb(119, 34, 180));

  game_set_scene(g, (Scene){
                        .context = gs,
                        .update = (SceneUpdateCB)GameScene_update,
                        .draw = (SceneDrawCB)GameScene_draw,
                        .mouse_move = (SceneMouseMoveCB)GameScene_mouse_move,
                        .mouse_click = (SceneClickCB)GameScene_click,
                    });
}