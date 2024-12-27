
#include "game/GameScene.h"
#include "game/Game.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/Marketplace.h"
#include "game/SceneObject.h"
#include "game/StreetMap.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math.h"
#include "math/Rect.h"
#include "math/Vec2.h"

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
  (void)g;
  gs->daytime += dt / 60.0f;
  if (gs->daytime > 1.0f) {
    gs->daytime -= 1.0f;
    gs->day++;
  }

  for (int i = 0; i < gs->scene_objects.len; ++i)
    SceneObject_update(&gs->scene_objects.data[i], gs, dt);

  SceneObjectVec_filter_dead(&gs->scene_objects);
}

void GameScene_draw(GameScene *gs, Game *g) {

  StreetMap_draw(gs->street_map, g);

  for (int i = 0; i < gs->scene_objects.len; ++i)
    SceneObject_draw(&gs->scene_objects.data[i], g);

  if (gs->menu_under_mouse < 0 && Level_validR(gs->level, gs->r)) {
    if (gs->r.h > 0 && gs->r.w > 0) {
      g_color(g, gs->preview);
      g_buffer(g, g_tilerect_buffer(g, gs->r.w, gs->r.h), Img_house_map, Level_to_vec(gs->r.x, gs->r.y));
    }
    g_color(g, red());
    for (int i = gs->r.x; i < gs->r.x + gs->r.w; ++i)
      for (int j = gs->r.y; j < gs->r.y + gs->r.h; ++j)
        g_object(g, g_animation_buffer(g), Img_marker, g_frame(g) % 4, Level_to_vec(i, j));
  }
}

void GameScene_draw_overlay(GameScene *gs, Game *g) {
  g_color(g, white());
  for (int i = 0; i < 10; ++i)
    g_object(g, g_animation_buffer(g), Img_menubar, i % 16, (Vec2){8 + 4 + i * 16, 8 + 4});
  for (int i = 0; i < 10; ++i) {
    g_color(g, i == gs->menu_under_mouse ? red() : (gs->menu_selected == i ? green() : blue()));
    g_object(g, g_animation_buffer(g), Img_marker, i == gs->menu_under_mouse ? g_frame(g) % 4 : i % 4,
             (Vec2){8 + 4 + i * 16, 8 + 4});
  }

  Size vp = g_viewport(g);
  Vec2 clock_pos = (Vec2){vp.w - 16.0f, vp.h - 16.0f};
  g_color(g, gs->daytime > 0.75 ? red() : white());
  g_objectRS(g, g_animation_buffer(g), Img_overlay_images, 0, clock_pos, -gs->daytime * M_PI * 2.0f, 2.0f);
  g_objectS(g, g_animation_buffer(g), Img_overlay_images, 1, clock_pos, 2.0f);
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

PathPoint *path = NULL;
Point start = (Point){-1, -1};
Point stop = (Point){-1, -1};
bool reached_goal(GameScene *gs, int i, int j) {
  (void)gs;
  return i == stop.x && j == stop.y;
}
bool movable(GameScene *gs, int i, int j) { return Level_movable(gs->level, i, j); }
void mark_path(GameScene *gs, int i, int j) {
  PathPoint *pp = gc_malloc(&gc, sizeof(PathPoint));
  *pp = (PathPoint){Level_to_vec(i, j), path};
  path = pp;
  Level_set_tile(gs->level, i, j, T_Path);
}

void GameScene_mouse_down(GameScene *gs, Game *g, Vec2 mp, Vec2 op, int button) {
  (void)g;
  (void)mp;
  (void)op;

  if (button == 0) {
    if (gs->menu_under_mouse >= 0) {
      gs->menu_selected = gs->menu_under_mouse;
      if (gs->menu_selected == 0) {
        gs->preview = Street_color();
        gs->r.w = gs->r.h = 1;
      } else if (gs->menu_selected == 1) {
        gs->preview = House_color();
        gs->r.w = gs->r.h = 2;
      } else if (gs->menu_selected == 2) {
        gs->preview = Marketplace_color();
        gs->r.w = 4;
        gs->r.h = 3;
      } else {
        gs->r.w = gs->r.h = 0;
      }
    } else if (gs->menu_selected == 0) {
      Level_set_movable(gs->level, gs->r.x, gs->r.y, true);
      StreetMap_update(gs->street_map);
    } else if (gs->menu_selected == 1) {
      House_init(g, gs, (Point){gs->r.x, gs->r.y});
    } else if (gs->menu_selected == 2) {
      Marketplace_init(g, gs, (Point){gs->r.x, gs->r.y});
    } else {
      if (Level_movable(gs->level, gs->r.x, gs->r.y)) {
        if (start.x < 0) {
          Level_clear_paths(gs->level);
          start = (Point){gs->r.x, gs->r.y};
          gs->w->position = gs->w->destination = Level_to_vecP(start);
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
          gs->w->path = path;
          path = NULL;
        }
        Level_set_tile(gs->level, gs->r.x, gs->r.y, T_PathStartEnd);
      }
    }
  }
}

void GameScene_add_object(GameScene *gs, SceneObject so) { SceneObjectVec_push(&gs->scene_objects, so); }

SceneTable GameScene_table = {
    .update = (SceneUpdateCB)GameScene_update,
    .draw = (SceneDrawCB)GameScene_draw,
    .draw_overlay = (SceneDrawCB)GameScene_draw_overlay,
    .mouse_move = (SceneMouseMoveCB)GameScene_mouse_move,
    .mouse_down = (SceneMouseCB)GameScene_mouse_down,
};
void GameScene_init(Game *g) {
  GameScene *gs = g_malloc(g, sizeof(GameScene));
  *gs = (GameScene){.scene_objects = (SceneObjectVec){NULL, 0, 0},
                    .menu_under_mouse = -1,
                    .menu_selected = 0,
                    .level = g_malloc(g, sizeof(Level)),
                    .r = (Recti){-1, -1, 1, 1}};

  Level_init(gs->level);
  gs->street_map = StreetMap_init(g, gs);

  gs->w = Wearisome_init(g, gs, (Vec2){0, 0});

  g_set_scene(g, (Scene){gs, &GameScene_table});
}