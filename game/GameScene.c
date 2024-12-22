
#include "game/GameScene.h"
#include "game/Game.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/StreetMap.h"
#include "game/Wearisome.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdint.h>

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

void GameScene_update(GameScene *gs, Game *g, float dt) {
  for (int i = 0; i < gs->scene_objects.len; ++i)
    SceneObject_update(&gs->scene_objects.data[i], g, gs, dt);

  SceneObjectVec_filter_dead(&gs->scene_objects);
}

void GameScene_draw(GameScene *gs, Game *g) {
  g_noise(g, 0.0f);
  g_color(g, white());
  // g_buffer(g, g_tilemap_buffer(g), gs->tilemap_img, (Vec2){8, 8});

  StreetMap_draw(gs->street_map, g);

  for (int i = 0; i < gs->scene_objects.len; ++i)
    SceneObject_draw(&gs->scene_objects.data[i], g);

  if (gs->menu_under_mouse < 0 && Level_validR(gs->level, gs->r)) {
    g_noise(g, 0.0f);
    g_color(g, red());
    for (int i = gs->r.x; i < gs->r.x + gs->r.w; ++i)
      for (int j = gs->r.y; j < gs->r.y + gs->r.h; ++j)
        g_object(g, g_animation_buffer(g), gs->marker, Level_to_vec(i, j), 0.0f, g_frame(g) % 4);
  }
}

void GameScene_draw_overlay(GameScene *gs, Game *g) {
  g_noise(g, 0.0f);
  g_color(g, white());
  for (int i = 0; i < 10; ++i)
    g_object(g, g_animation_buffer(g), gs->menubar_img, (Vec2){8 + 4 + i * 16, 8 + 4}, 0.0f, i % 16);
  for (int i = 0; i < 10; ++i) {
    g_noise(g, i == gs->menu_under_mouse ? 0.3f : 0.0f);
    g_color(g, i == gs->menu_under_mouse ? red() : (gs->menu_selected == i ? green() : blue()));
    g_object(g, g_animation_buffer(g), gs->marker, (Vec2){8 + 4 + i * 16, 8 + 4}, 0.0f,
             i == gs->menu_under_mouse ? g_frame(g) % 4 : i % 4);
  }
}

void GameScene_mouse_move(GameScene *gs, Game *g, Vec2 mp, Vec2 op) {
  (void)g;
  gs->r.x = (int)((mp.x + 8) / 16.0f);
  gs->r.y = (int)((mp.y + 8) / 16.0f);

  gs->menu_under_mouse = -1;
  for (int i = 0; i < 10; ++i)
    if (Rect_contains((Rect){(Vec2){4 + i * 16, 4}, (Vec2){16, 16}}, op))
      gs->menu_under_mouse = i;
}

Point start = (Point){-1, -1};
Point stop = (Point){-1, -1};
bool reached_goal(GameScene *gs, int i, int j) {
  (void)gs;
  return i == stop.x && j == stop.y;
}
bool movable(GameScene *gs, int i, int j) { return Level_movable(gs->level, i, j); }
void mark_path(GameScene *gs, int i, int j) { Level_set_tile(gs->level, i, j, T_Path); }

void GameScene_mouse_down(GameScene *gs, Game *g, Vec2 mp, Vec2 op, int button) {
  (void)g;
  (void)mp;
  (void)op;

  if (button == 0) {
    if (gs->menu_under_mouse >= 0) {
      gs->menu_selected = gs->menu_under_mouse;
      if (gs->menu_selected == 0) {
        gs->r.w = gs->r.h = 1;
      } else if (gs->menu_selected == 1) {
        gs->r.w = gs->r.h = 2;
      } else {
        gs->r.w = gs->r.h = 0;
      }
    } else if (gs->menu_selected == 0) {
      Level_set_movable(gs->level, gs->r.x, gs->r.y, true);
    } else {
      if (Level_movable(gs->level, gs->r.x, gs->r.y)) {
        if (start.x < 0) {
          Level_clear_paths(gs->level);
          start = (Point){gs->r.x, gs->r.y};
        } else {
          stop = (Point){gs->r.x, gs->r.y};
          bfs(gs->level, start.x, start.y,
              (SearchHandle){
                  gs,
                  (CanMoveCB)movable,
                  (GoalReachedCB)reached_goal,
                  (PathCB)mark_path,
              });
          start = (Point){-1, -1};
        }
        Level_set_tile(gs->level, gs->r.x, gs->r.y, T_PathStartEnd);
      }
    }
  }
}

void GameScene_add_object(GameScene *gs, SceneObject so) { SceneObjectVec_push(&gs->scene_objects, so); }

void GameScene_init(Game *g) {
  GameScene *gs = gc_malloc(&gc, sizeof(GameScene));
  *gs = (GameScene){.scene_objects = (SceneObjectVec){NULL, 0, 0},
                    .tilemap_img = g_image(g, Img_tilemap),
                    .menubar_img = g_image(g, Img_menubar),
                    .marker = g_image(g, Img_marker),
                    .menu_under_mouse = -1,
                    .menu_selected = 0,
                    .level = gc_malloc(&gc, sizeof(Level)),
                    .r = (Recti){-1, -1, 1, 1}};

  Level_init(gs->level);
  gs->street_map = StreetMap_init(g, gs);

  Wearisome_init(g, gs, (Vec2){18 * 16, 14 * 16}, Evil);
  Wearisome_init(g, gs, (Vec2){1 * 16, 1 * 16}, Good);

  g_set_scene(g, (Scene){
                     .context = gs,
                     .update = (SceneUpdateCB)GameScene_update,
                     .draw = (SceneDrawCB)GameScene_draw,
                     .draw_overlay = (SceneDrawCB)GameScene_draw_overlay,
                     .mouse_move = (SceneMouseMoveCB)GameScene_mouse_move,
                     .mouse_down = (SceneMouseCB)GameScene_mouse_down,
                 });
}