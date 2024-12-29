#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Circ.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include "math/random.h"

typedef struct PathPoint {
  Vec2 p;
  struct PathPoint *next;
} PathPoint;

PathPoint *PathPoint_init(Vec2 p, PathPoint *next) {
  PathPoint *pp = gc_malloc(&gc, sizeof(PathPoint));
  *pp = (PathPoint){p, next};
  return pp;
}

typedef enum WearisomeState {
  W_None = 0,
  W_AtHome,
  W_MovingHome,
  W_Waiting,
  W_Wandering,
} WearisomeState;

typedef struct Wearisome {
  Recti home;
  Vec2 position, destination;

  PathPoint *path;
  float wait_time;

  struct {
    float food, water, sleep;
  } needs;
  float health;

  WearisomeState state;
} Wearisome;

bool w_dead(Wearisome *w) { return w->health <= 0.0f; }

float w_render_order(Wearisome *w) { return 10000.0f + w->position.y; }

bool w_move_to(Wearisome *w, GameScene *gs, Recti cur, Recti dest);
bool w_move_to_rect(Wearisome *w, GameScene *gs, Recti r) { return w_move_to(w, gs, (Recti){0}, r); }

void w_wander_to_random_near_path(Wearisome *w, GameScene *gs) {
  Point l = Level_to_point(w->destination);
  for (int i = 0; i < 1000; i++) {
    int i = l.x + (rand() % 10) - 5;
    int j = l.y + (rand() % 10) - 5;
    if (Level_movable(gs->level, i, j)) {
      if (w_move_to(w, gs, w->state == W_AtHome ? w->home : (Recti){0}, (Recti){i, j, 1, 1}))
        w->state = W_Wandering;
      break;
    }
  }
}

void w_u_waiting(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (gs->daytime > 0.75f && w_move_to_rect(w, gs, w->home)) {
    w->state = W_MovingHome;
  } else if (w->wait_time < 0.0f) {
    w_wander_to_random_near_path(w, gs);
  }
}

void w_u_moving_home(Wearisome *w, float dt) {
  w->position = v_lerp_about(w->position, w->destination, dt * 32.0);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else
      w->state = W_AtHome;
  }
}

void w_u_wandering(Wearisome *w, GameScene *gs, float dt) {
  w->position = v_lerp_about(w->position, w->destination, dt * 48.0);
  if (v_eq(w->position, w->destination)) {
    if (gs->daytime > 0.75f && w_move_to_rect(w, gs, w->home)) {
      w->state = W_MovingHome;
    } else if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else {
      w->wait_time = r_float_r(1.25f, 3.4f);
      w->state = W_Waiting;
    }
  }
}

void w_update(Wearisome *w, GameScene *gs, float dt) {
  if (w_dead(w))
    return;

  w->needs.water = f_max(0.0f, w->needs.water - gs->daytime_step / 2.0f);
  w->needs.food = f_max(0.0f, w->needs.food - gs->daytime_step / 4.0f);
  w->needs.sleep = f_max(0.0f, w->needs.sleep - gs->daytime_step / 2.0f);
  if (w->needs.water < 0.1f || w->needs.food < 0.1f || w->needs.sleep < 0.1f)
    w->health -= 2.0f * gs->daytime_step;
  else
    w->health = f_min(1.0f, w->health + gs->daytime_step / 8.0f);

  // printf("Wearisome: %f (w:%f f:%f s:%f)\n", w->health, w->needs.water, w->needs.food, w->needs.sleep);

  switch (w->state) {
  case W_None:
    break;

  case W_AtHome: {
    const float ref = 1.0f - f_min(w->needs.sleep, w->health);
    if (gs->daytime > ref && gs->daytime < 0.75f) {
      w->path = NULL;
      w_wander_to_random_near_path(w, gs);
      if (!w->path)
        w->state = W_AtHome;
    }
    break;
  }

  case W_MovingHome:
    w_u_moving_home(w, dt);
    break;

  case W_Waiting:
    w_u_waiting(w, gs, dt);
    break;

  case W_Wandering:
    w_u_wandering(w, gs, dt);
    break;
  }
}

void w_draw(Wearisome *w, Game *g) {
  g_color(g, white());
  g_objectS(g, g_animation_buffer(g), Img_weapons, 0, v_add(w->position, (Vec2){8, 4}), 2.0f);

  Vec2 p = v_add(w->position, (Vec2){0, 2});
  g_color(g, w_dead(w) ? rgb(0, 0, 0) : warn(w->health));
  g_object(g, g_animation_buffer(g), Img_wearisome, w_dead(w) ? 0 : g_frame(g) % 4, p);

  g_color(g, warn(w->needs.water));
  g_objectS(g, g_animation_buffer(g), Img_maze_pointer, 4, v_add(w->position, (Vec2){-4, 12}), 0.25);
  g_color(g, warn(w->needs.food));
  g_objectS(g, g_animation_buffer(g), Img_maze_pointer, 4, v_add(w->position, (Vec2){0, 12}), 0.25);
  g_color(g, warn(w->needs.sleep));
  g_objectS(g, g_animation_buffer(g), Img_maze_pointer, 4, v_add(w->position, (Vec2){4, 12}), 0.25);
}

typedef struct WearisomePathSearchData {
  Wearisome *w;
  GameScene *gs;
  Point start;
  Recti start_rect;
  Recti destination;
  PathPoint *path;
} WearisomePathSearchData;

bool WearisomePathSearch_moveable(WearisomePathSearchData *data, int x, int y) {
  return Level_movable(data->gs->level, x, y) || ri_contains(data->destination, x, y) ||
         ri_contains(data->start_rect, x, y);
}

bool WearisomePathSearch_reached_goal(WearisomePathSearchData *data, int x, int y) {
  return ri_contains(data->destination, x, y);
}

void WearisomePathSearch_build_path(WearisomePathSearchData *data, int i, int j) {
  data->path = PathPoint_init(Level_to_vec(i, j), data->path);
}

bool w_move_to(Wearisome *w, GameScene *gs, Recti cur, Recti dest) {
  WearisomePathSearchData search_data = {w, gs, Level_to_point(w->destination), cur, dest, NULL};
  bfs(gs->level, search_data.start.x, search_data.start.y,
      (SearchHandle){
          &search_data,
          (CanMoveCB)WearisomePathSearch_moveable,
          (GoalReachedCB)WearisomePathSearch_reached_goal,
          (PathCB)WearisomePathSearch_build_path,
      });
  w->path = search_data.path;
  return w->path != NULL;
}

bool w_is_home(Wearisome *w) { return w->state == W_AtHome; }

float apply_need(float *n, float t) {
  *n += t;
  float r = f_max(0.0, *n - 1.0f);
  *n -= r;
  return t - r;
}

void w_sleep(Wearisome *w, float t) { w->needs.sleep = f_min(1.0f, w->needs.sleep + t); }
float w_drink(Wearisome *w, float t) { return apply_need(&w->needs.water, t); }
float w_eat(Wearisome *w, float t) { return apply_need(&w->needs.food, t); }

SceneObjectTable w_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)w_dead,
    .render_order = (SceneObjectRenderOrderCB)w_render_order,
    .update = (SceneObjectUpdateCB)w_update,
    .draw = (SceneObjectDrawCB)w_draw,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, Recti home) {
  Vec2 pos = Level_to_vec(home.x, home.y);
  Wearisome *w = g_malloc(g, sizeof(Wearisome));
  *w = (Wearisome){
      .home = home,
      .position = pos,
      .destination = pos,
      .path = NULL,
      .needs = {.food = 1.0f, .water = 1.0f, .sleep = 1.0f},
      .health = 1.0f,
      .state = W_AtHome,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &w_table});
  return w;
}

#endif