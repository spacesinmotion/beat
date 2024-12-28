#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Circ.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include "math/random.h"
#include <stdlib.h>
#include <time.h>

typedef struct PathPoint {
  Vec2 p;
  struct PathPoint *next;
} PathPoint;

PathPoint *PathPoint_init(Vec2 p, PathPoint *next) {
  PathPoint *pp = gc_malloc(&gc, sizeof(PathPoint));
  *pp = (PathPoint){p, next};
  return pp;
}

PathPoint *PathPoint_append(PathPoint *pp, Vec2 dest) {
  if (!pp)
    return PathPoint_init(dest, NULL);

  PathPoint *p = pp;
  while (p->next)
    p = p->next;
  p->next = PathPoint_init(dest, NULL);
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

  WearisomeState state;
} Wearisome;

bool w_dead(Wearisome *w) {
  (void)w;
  return false;
}

Circle w_circle(Wearisome *w) { return (Circle){w->position, 8.0f}; }

void w_move_to(Wearisome *w, GameScene *gs, Point d, WearisomeState new_state);
void w_move_to_rect(Wearisome *w, GameScene *gs, Recti r, WearisomeState new_state) {
  w_move_to(w, gs, Level_movable_around(gs->level, r), new_state);
  w->path = PathPoint_append(w->path, Level_to_vec(r.x, r.y));
}

void w_u_waiting(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (gs->daytime > 0.75f) {
    w_move_to_rect(w, gs, w->home, W_MovingHome);
  } else if (w->wait_time < 0.0f) {
    Point l = Level_to_point(w->destination);
    for (int i = 0; i < 1000; i++) {
      int i = l.x + (rand() % 10) - 5;
      int j = l.y + (rand() % 10) - 5;
      if (Level_movable(gs->level, i, j)) {
        w_move_to(w, gs, (Point){i, j}, W_Wandering);
        break;
      }
    }
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
  w->position = v_lerp_about(w->position, w->destination, dt * 32.0);
  if (v_eq(w->position, w->destination)) {
    if (gs->daytime > 0.75f) {
      w_move_to_rect(w, gs, w->home, W_MovingHome);
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
  switch (w->state) {
  case W_None:
    break;

  case W_AtHome:
    if (gs->daytime < 0.75f) {
      w->path = NULL;
      w->destination = Level_to_vecP(Level_movable_around(gs->level, w->home));
      w->state = W_Wandering;
    }
    break;

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
  g_color(g, rgb(77, 213, 30));
  g_object(g, g_animation_buffer(g), Img_wearisome, g_frame(g) % 4, p);
}

typedef struct WearisomePathSearchData {
  Wearisome *w;
  GameScene *gs;
  Point start;
  Point stop;
  PathPoint *path;
} WearisomePathSearchData;

bool WearisomePathSearch_moveable(WearisomePathSearchData *data, int x, int y) {
  return Level_movable(data->gs->level, x, y);
}
bool WearisomePathSearch_reached_goal(WearisomePathSearchData *data, int x, int y) {
  return x == data->stop.x && y == data->stop.y;
}

void WearisomePathSearch_build_path(WearisomePathSearchData *data, int i, int j) {
  data->path = PathPoint_init(Level_to_vec(i, j), data->path);
}

void w_move_to(Wearisome *w, GameScene *gs, Point d, WearisomeState new_state) {
  WearisomePathSearchData search_data = {w, gs, Level_to_point(w->destination), d, NULL};
  bfs(gs->level, search_data.start.x, search_data.start.y,
      (SearchHandle){
          &search_data,
          (CanMoveCB)WearisomePathSearch_moveable,
          (GoalReachedCB)WearisomePathSearch_reached_goal,
          (PathCB)WearisomePathSearch_build_path,
      });
  w->path = search_data.path;
  w->state = new_state;
}

bool w_is_home(Wearisome *w) { return w->state == W_AtHome; }

SceneObjectTable w_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)w_dead,
    .circle = (SceneObjectCircle)w_circle,
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
      .state = W_AtHome,
  };
  GameScene_add_object(gs, (SceneObject){.context = w, &w_table});
  return w;
}

#endif