#ifndef WEARISOME
#define WEARISOME

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"
#include "gc/gc.h"
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

  W_DeliverCollect,
  W_DeliverWait,
  W_Deliver,

  W_MoveToWork,
  W_Working,
} WearisomeState;

const char *WearisomeState_name(WearisomeState s) {
  switch (s) {
  case W_None:
    return "none";
  case W_AtHome:
    return "at home";
  case W_MovingHome:
    return "moving home";
  case W_Waiting:
    return "waiting";
  case W_Wandering:
    return "wandering";
  case W_DeliverCollect:
    return "collect something";
  case W_DeliverWait:
    return "wait for deliver";
  case W_Deliver:
    return "deliver something";
  case W_MoveToWork:
    return "move to work";
  case W_Working:
    return "working";
  }
  return "<error>";
}

typedef struct Needs {
  float food, water, sleep;
} Needs;

typedef bool (*CollectDoneCB)(void *, GameScene *);
typedef void (*DeliverDoneCB)(void *, GameScene *);
typedef struct DeliverJob {
  void *context;
  CollectDoneCB collect_done;
  DeliverDoneCB deliver_done;
  Recti from, to;
} DeliverJob;

DeliverJob *deliver_job(Recti from, Recti to, void *context, CollectDoneCB on_collect, DeliverDoneCB on_delivered) {
  DeliverJob *job = gc_malloc(&gc, sizeof(DeliverJob));
  *job = (DeliverJob){context, on_collect, on_delivered, from, to};
  return job;
}

bool dj_on_collect(DeliverJob *dj, GameScene *gs) { return !dj->collect_done || dj->collect_done(dj->context, gs); }
void dj_on_delivered(DeliverJob *dj, GameScene *gs) {
  if (dj->deliver_done)
    dj->deliver_done(dj->context, gs);
}

typedef struct Wearisome {
  House *home;
  Recti current_rect;
  Vec2 position, destination;

  PathPoint *path;
  float wait_time;

  Needs needs;
  Needs need_consumption;
  float health;
  bool needs_click;

  float speed;

  WearisomeState state;

  DeliverJob *deliver_job;
} Wearisome;

float apply_need(float *n, float t) {
  *n += t;
  float r = f_max(0.0, *n - 1.0f);
  *n -= r;
  return t - r;
}

void w_sleep(Wearisome *w, float t) { w->needs.sleep = f_min(1.0f, w->needs.sleep + t); }
float w_drink(Wearisome *w, float t) { return apply_need(&w->needs.water, t); }
float w_eat(Wearisome *w, float t) { return apply_need(&w->needs.food, t); }

bool w_dead(Wearisome *w) { return w->health <= 0.0f; }

float w_render_order(Wearisome *w) { return 10000.0f + w->position.y; }

bool w_move_to(Wearisome *w, GameScene *gs, Recti cur, Recti dest);
bool w_move_to_rect(Wearisome *w, GameScene *gs, Recti r) { return w_move_to(w, gs, w->current_rect, r); }

void w_wander_to_random_near_path(Wearisome *w, GameScene *gs) {
  Point l = l_to_point(w->destination);
  for (int i = 0; i < 1000; i++) {
    int i = l.x + (rand() % 8) - 4;
    int j = l.y + (rand() % 8) - 4;
    if (l_movable(gs->level, i, j)) {
      if (w_move_to(w, gs, w->current_rect, (Recti){i, j, 1, 1}))
        w->state = W_Wandering;
      return;
    }
  }

  printf("FAILED TO WANDER!!!\n");
}

typedef struct WearisomeJobSearchData {
  GameScene *gs;
  Recti start_rect;
  PathPoint *path;
} WearisomeJobSearchData;

bool WearisomeJobSearch_moveable(WearisomeJobSearchData *data, int x, int y) {
  if (l_movable(data->gs->level, x, y) || ri_contains(data->start_rect, x, y))
    return true;
  return tc_has_work(l_content(data->gs->level, x, y), data->gs);
}

bool WearisomeJobSearch_reached_goal(WearisomeJobSearchData *data, int x, int y) {
  TileContent *c = l_content(data->gs->level, x, y);
  if (!tc_has_work(c, data->gs))
    return false;
  tc_claim_work(c, data->gs);
  return true;
}

void WearisomeJobSearch_build_path(WearisomeJobSearchData *data, int i, int j) {
  data->path = PathPoint_init(l_to_vec(i, j), data->path);
}

bool w_has_emergency(Wearisome *w, float k) { return w->needs.sleep < k || w->needs.water < k || w->needs.food < k; }

void w_u_at_home(Wearisome *w, GameScene *gs, float dt) {
  w_sleep(w, 8.0f * gs->daytime_step);
  w->home->resources.water -= w_drink(w, f_min(w->home->resources.water, 12.0 * gs->daytime_step));
  w->home->resources.food -= w_eat(w, f_min(w->home->resources.food, 12.0 * gs->daytime_step));

  if (gs->daytime > 0.75 || w->needs.sleep < 0.7f || (w->needs.water < 0.7f && w->home->resources.water > 0.0f) ||
      (w->needs.food < 0.7f && w->home->resources.food > 0.0f))
    return;

  w->path = NULL;
  w_wander_to_random_near_path(w, gs);
  if (!w->path) {
    w->state = W_AtHome;
    w->current_rect = w->home->location;
  }
}

void w_u_moving_home(Wearisome *w, float dt) {
  w->position = v_lerp_about(w->position, w->destination, dt * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else {
      w->state = W_AtHome;
      w->current_rect = w->home->location;
    }
  }
}

bool w_want_to_work(Wearisome *w, GameScene *gs) {
  int key = w->needs_click ? 4 : 0;
  if (w->needs.water < 0.4)
    key--;
  if (w->needs.food < 0.4)
    key--;
  if (w->needs.sleep < 0.4)
    key--;
  if (w->needs.sleep < 0.5 && gs->daytime > 0.55)
    key--;
  if (w->needs.sleep < 0.6 && gs->daytime > 0.65)
    key--;
  if (w->needs.sleep < 0.8 && gs->daytime > 0.7)
    key--;
  return rand() % 6 <= key;
}

DeliverJob *h_deliver_job(House *h, GameScene *gs) {
  if (h->resources_maximum.clicks <= 0)
    return NULL;

  bool need_water = h->resources_maximum.water - h->resources.water >= 1.0f;
  bool need_food = h->resources_maximum.food - h->resources.food >= 1.0f;
  bool food_is_more_urgent = need_water && need_food && h->resources.water > h->resources.food;
  if (!food_is_more_urgent && need_water && (gs->resource_pool.water - gs->resource_pool_claimed.water > 0)) {
    gs->resource_pool_claimed.water++;
    h->resources_maximum.clicks--;
    return deliver_job((Recti){17, 10, 4, 3}, h->location, h, (CollectDoneCB)h_pay_water,
                       (DeliverDoneCB)h_get_water_done);
  } else if (need_food && (gs->resource_pool.food - gs->resource_pool_claimed.food > 0)) {
    gs->resource_pool_claimed.food++;
    h->resources_maximum.clicks--;
    return deliver_job((Recti){17, 10, 4, 3}, h->location, h, (CollectDoneCB)h_pay_food,
                       (DeliverDoneCB)h_get_food_done);
  }
  return NULL;
}

bool w_deliver(Wearisome *w, GameScene *gs, DeliverJob *job);

bool w_check_what_to_do_next(Wearisome *w, GameScene *gs) {
  WearisomeState old_state = w->state;
  DeliverJob *job = NULL;

  if (gs->daytime > 0.75f && w_move_to_rect(w, gs, w->home->location))
    w->state = W_MovingHome;

  else if (w->needs.sleep < 0.25f && w_move_to_rect(w, gs, w->home->location))
    w->state = W_MovingHome;

  else if (((w->needs.water < 0.25f && w->home->resources.water > 0.0f) ||
            (w->needs.food < 0.25f && w->home->resources.food > 0.0f)) &&
           w_move_to_rect(w, gs, w->home->location))
    w->state = W_MovingHome;

  else if ((job = h_deliver_job(w->home, gs)))
    w_deliver(w, gs, job);

  else if (w->home->resources_maximum.clicks < 2 || rand() % 10 < 3) {
    WearisomeJobSearchData search_data = {gs, w->current_rect, NULL};
    Point start = l_to_point(w->destination);
    l_bright_first(gs->level, start.x, start.y,
                   (SearchHandle){
                       &search_data,
                       (CanMoveCB)WearisomeJobSearch_moveable,
                       (GoalReachedCB)WearisomeJobSearch_reached_goal,
                       (PathCB)WearisomeJobSearch_build_path,
                   });
    if (search_data.path) {
      w->path = search_data.path;
      w->state = W_MoveToWork;
    }
  }

  return w->state != old_state;
}

void w_u_waiting(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (w->wait_time < 0.0f && !w_check_what_to_do_next(w, gs))
    w_wander_to_random_near_path(w, gs);
}

void w_u_wandering(Wearisome *w, GameScene *gs, float dt) {
  w->position = v_lerp_about(w->position, w->destination, dt * 0.334 * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (!w_check_what_to_do_next(w, gs)) {
      if (w->path) {
        w->destination = w->path->p;
        w->path = w->path->next;
      } else {
        w->wait_time = r_float_r(1.25f, 3.4f);
        w->state = W_Waiting;
      }
    }
  }
}

void w_u_deliver_collect(Wearisome *w, GameScene *gs, float dt) {
  (void)gs;

  w->position = v_lerp_about(w->position, w->destination, dt * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else {
      w->wait_time = 0.5f;
      w->state = W_DeliverWait;
      w->current_rect = w->deliver_job->from;
    }
  }
}

void w_u_deliver_wait(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (!w->deliver_job)
    w->state = W_Waiting;
  else if (w->wait_time < 0.0f) {
    if (dj_on_collect(w->deliver_job, gs) && w_move_to_rect(w, gs, w->deliver_job->to)) {
      w->state = W_Deliver;
    } else {
      w->state = W_Waiting;
      w->deliver_job = NULL;
    }
  }
}

void w_u_deliver(Wearisome *w, GameScene *gs, float dt) {
  w->position = v_lerp_about(w->position, w->destination, dt * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else {
      w->current_rect = w->deliver_job->to;
      dj_on_delivered(w->deliver_job, gs);
      w->deliver_job = NULL;
      w_wander_to_random_near_path(w, gs);
    }
  }
}

void w_u_move_to_work(Wearisome *w, GameScene *gs, float dt) {
  (void)gs;

  w->position = v_lerp_about(w->position, w->destination, dt * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else {
      w->wait_time = tc_start_work(l_contentP(gs->level, l_to_point(w->destination)), gs);
      w->state = W_Working;

      // w->current_rect = w->deliver_job->from;
    }
  }
}

void w_u_working(Wearisome *w, GameScene *gs, float dt) {
  (void)gs;

  w->wait_time -= dt;
  if (w->wait_time < 0.0f) {
    tc_done_work(l_contentP(gs->level, l_to_point(w->destination)), gs);
    h_earn_click(w->home, 1);
    w_wander_to_random_near_path(w, gs);
  }
}

void w_update(Wearisome *w, GameScene *gs, float dt) {
  if (w_dead(w))
    return;

  const float working_factor = w->state == W_Working ? 1.25 : 1.0;
  w->needs.water = f_max(0.0f, w->needs.water - gs->daytime_step * w->need_consumption.water * working_factor);
  w->needs.food = f_max(0.0f, w->needs.food - gs->daytime_step * w->need_consumption.food * working_factor);
  w->needs.sleep = f_max(0.0f, w->needs.sleep - gs->daytime_step * w->need_consumption.sleep * working_factor);
  if (w->needs.water < 0.1f || w->needs.food < 0.1f || w->needs.sleep < 0.1f)
    w->health -= 2.0f * gs->daytime_step;
  else
    w->health = f_min(1.0f, w->health + gs->daytime_step / 8.0f);

  w->home->wearisome_dead = w->health <= 0.0;

  Point p = l_to_point(w->destination);
  if (!ri_contains(w->current_rect, p.x, p.y))
    w->current_rect = (Recti){0};

  switch (w->state) {
  case W_None:
    break;

  case W_AtHome: {
    w_u_at_home(w, gs, dt);
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

  case W_DeliverCollect:
    w_u_deliver_collect(w, gs, dt);
    break;
  case W_DeliverWait:
    w_u_deliver_wait(w, gs, dt);
    break;
  case W_Deliver:
    w_u_deliver(w, gs, dt);
    break;

  case W_MoveToWork:
    w_u_move_to_work(w, gs, dt);
    break;
  case W_Working:
    w_u_working(w, gs, dt);
    break;
  }
}

void w_draw(Wearisome *w, GameScene *gs, Game *g) {
  (void)gs;
  if (w->home->highlight) {
    g_color(g, rgb(200, 62, 235));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(w->position, (Vec2){0, 8}), 0.75f);

    c_printf(g, "----------------------\n");
    c_printf(g, " %10s: %s\n", "state", WearisomeState_name(w->state));
    c_printf(g, " %10s: %f\n", "health", w->health);
    c_printf(g, " %10s: %f\n", "sleep", w->needs.sleep);
    c_printf(g, " %10s: %f\n", "water", w->needs.water);
    c_printf(g, " %10s: %f\n", "food", w->needs.food);
  }

  if (w->state == W_Working || w->state == W_AtHome)
    return;
  Vec2 p = v_add(w->position, (Vec2){0, 2});
  g_color(g, w_dead(w) ? rgb(0, 0, 0) : warn(w->health));
  const int o = (size_t)w / 17;
  g_object(g, g_animation_buffer(g), Img_wearisome, w_dead(w) ? 0 : (o + g_frame(g)) % 4, p);

  if (w->needs.water < 0.75) {
    g_color(g, warn(w->needs.water));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(w->position, (Vec2){-4, 12}), 0.25);
  }
  if (w->needs.food < 0.75) {
    g_color(g, warn(w->needs.food));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(w->position, (Vec2){0, 12}), 0.25);
  }
  if (w->needs.sleep < 0.75) {
    g_color(g, warn(w->needs.sleep));
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(w->position, (Vec2){4, 12}), 0.25);
  }
}

typedef struct WearisomePathSearchData {
  GameScene *gs;
  Recti start_rect;
  Recti destination;
  PathPoint *path;
} WearisomePathSearchData;

bool WearisomePathSearch_moveable(WearisomePathSearchData *data, int x, int y) {
  return l_movable(data->gs->level, x, y) || ri_contains(data->destination, x, y) ||
         ri_contains(data->start_rect, x, y);
}

bool WearisomePathSearch_reached_goal(WearisomePathSearchData *data, int x, int y) {
  return ri_contains(data->destination, x, y);
}

void WearisomePathSearch_build_path(WearisomePathSearchData *data, int i, int j) {
  data->path = PathPoint_init(l_to_vec(i, j), data->path);
}

bool w_move_to(Wearisome *w, GameScene *gs, Recti cur, Recti dest) {
  WearisomePathSearchData search_data = {gs, cur, dest, NULL};
  Point start = l_to_point(w->destination);
  l_bright_first(gs->level, start.x, start.y,
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

bool w_is_free(Wearisome *w) { return w->state == W_Wandering || w->state == W_Waiting; }
bool w_deliver(Wearisome *w, GameScene *gs, DeliverJob *job) {
  if (w_move_to_rect(w, gs, job->from)) {
    w->deliver_job = job;
    w->state = W_DeliverCollect;
    return true;
  }
  return false;
}

SceneObjectTable w_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)w_dead,
    .render_order = (SceneObjectRenderOrderCB)w_render_order,
    .update = (SceneObjectUpdateCB)w_update,
    .draw = (SceneObjectDrawCB)w_draw,
};
Wearisome *Wearisome_init(Game *g, GameScene *gs, House *home) {
  Vec2 pos = l_to_vec(home->location.x, home->location.y);
  Wearisome *w = g_malloc(g, sizeof(Wearisome));
  *w = (Wearisome){
      .home = home,
      .current_rect = home->location,
      .position = pos,
      .destination = pos,
      .path = NULL,
      .needs = {.food = 1.0f, .water = 1.0f, .sleep = 1.0f},
      .need_consumption = {.food = r_float_r(0.2f, 0.3f),
                           .water = r_float_r(0.35f, 0.65f),
                           .sleep = r_float_r(0.35f, 0.65f)},
      .health = 1.0f,
      .deliver_job = NULL,
      .state = W_AtHome,
      .needs_click = false,
      .speed = r_float_r(60.0f, 70.0f),
  };
  gs_add_object(gs, (SceneObject){.context = w, &w_table});
  return w;
}

House *Wearisome_House_init(Game *g, GameScene *gs, Point p) {
  House *h = House_init(g, gs, p);
  Wearisome_init(g, gs, h);
  return h;
}

#endif