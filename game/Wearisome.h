#ifndef WEARISOME
#define WEARISOME

#include "game/Farm.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/House.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/Well.h"
#include "game/assets.h"
#include "game/jobs/DeliverJob.h"
#include "game/jobs/QueueItem.h"
#include "game/search/RectSearch.h"
#include "game/search/ResourceProviderSearch.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include "math/random.h"

typedef enum WearisomeState {
  W_None = 0,
  W_AtHome,
  W_MovingHome,
  W_Waiting,
  W_Wandering,

  W_DeliverCollect,
  W_DeliverWait,
  W_Deliver,

  W_WorkDeliverCollect,
  W_WorkDeliverWait,
  W_WorkDeliver,

  W_MoveToWork,
  W_Working,

  W_QueueMove,
  W_QueueWait,

  W_MoveToEntertainment,
  W_GetEntertained,
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
  case W_WorkDeliverCollect:
    return "collect something for work";
  case W_WorkDeliverWait:
    return "wait for deliver for work";
  case W_WorkDeliver:
    return "deliver something for work";
  case W_MoveToWork:
    return "move to work";
  case W_Working:
    return "working";
  case W_QueueMove:
    return "queue job move";
  case W_QueueWait:
    return "queue job wait";
  case W_MoveToEntertainment:
    return "move to entertainment";
  case W_GetEntertained:
    return "get entertained";
  }
  assert(false);
  return "<error>";
}

typedef struct Needs {
  float food, water, sleep;
} Needs;

typedef struct Wearisome {
  House *home;
  Recti current_building;
  Vec2 position, destination;

  PathPoint *path;
  float wait_time;

  Needs needs;
  Needs need_consumption;
  float health;

  float speed;

  QueueItem queue;
  WearisomeState state;

  DeliverJob *deliver_job;
} Wearisome;

static inline float apply_need(float *n, float t) {
  *n += t;
  float r = f_max(0.0, *n - 1.0f);
  *n -= r;
  return t - r;
}

static inline void w_sleep(Wearisome *w, float t) { w->needs.sleep = f_min(1.0f, w->needs.sleep + t); }
static inline float w_drink(Wearisome *w, float t) { return apply_need(&w->needs.water, t); }
static inline float w_eat(Wearisome *w, float t) { return apply_need(&w->needs.food, t); }

bool w_dead(Wearisome *w) { return w->health <= 0.0f; }

float w_render_order(Wearisome *w) { return 10000.0f + w->position.y; }

static inline Recti w_current_rect(Wearisome *w) {
  Point p = l_to_point(w->destination);
  return (Recti){p.x, p.y, 1, 1};
}

static inline bool w_move_to(Wearisome *w, GameScene *gs, Recti dest) {
  Point p = l_to_point(w->destination);
  w->path = find_path_from_rect_to_rect(gs, p, w->current_building, dest);
  return w->path != NULL;
}
static inline bool w_move_to_entertainment(Wearisome *w, GameScene *gs, Recti location) {
  if (!w_move_to(w, gs, location))
    return false;
  w->state = W_MoveToEntertainment;
  return true;
}

static inline bool w_move_to_work(Wearisome *w, GameScene *gs, Recti location) {
  if (!w_move_to(w, gs, location))
    return false;
  w->state = W_MoveToWork;
  return true;
}
static inline bool w_queue_move_to(Wearisome *w, GameScene *gs, Recti location, QueueItem qi) {
  if (!w_move_to(w, gs, location))
    return false;
  w->queue = qi;
  w->state = W_QueueMove;
  return true;
}
static inline bool w_queue_wait_for(Wearisome *w, float time, QueueItem qi) {
  w->wait_time = time;
  w->queue = qi;
  w->state = W_QueueWait;
  return true;
}

static inline void w_earn_clicks(Wearisome *w, int c) { h_earn_click(w->home, c); }

void w_wander_to_random_near_path(Wearisome *w, GameScene *gs) {
  Point l = l_to_point(w->destination);
  for (int i = 0; i < 1000; i++) {
    int i = l.x + (rand() % 8) - 4;
    int j = l.y + (rand() % 8) - 4;
    if (l_movable(gs->level, i, j)) {
      if (w_move_to(w, gs, (Recti){i, j, 1, 1}))
        w->state = W_Wandering;
      return;
    }
  }
}

bool w_has_emergency(Wearisome *w, float k) { return w->needs.sleep < k || w->needs.water < k || w->needs.food < k; }

void w_u_at_home(Wearisome *w, GameScene *gs, float dt) {
  (void)dt;

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
    w->current_building = w->home->display.location;
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
      w->current_building = w->home->display.location;
    }
  }
}

bool w_want_to_work(Wearisome *w, GameScene *gs) {
  int reduce_urgent = 4;
  if (gs->daytime > 0.5)
    reduce_urgent++;
  if (gs->daytime > 0.6)
    reduce_urgent++;
  if (gs->daytime > 0.7)
    reduce_urgent++;
  if (w->needs.sleep < 0.25f)
    reduce_urgent += 10;
  if (w->needs.water < 0.25f)
    reduce_urgent += 10;
  if (w->needs.food < 0.25f)
    reduce_urgent += 10;

  float ref = reduce_urgent * w->home->resources_maximum.clicks;
  ref = 1 / (ref + 1);
  return r_float() < ref;
}

bool w_want_entertainment(Wearisome *w) {
  if (w->needs.sleep < 0.25f || w->needs.water < 0.25f || w->needs.food < 0.25f)
    return false;
  float ref = w->home->resources_maximum.clicks;
  ref = ref * ref / 74.0f;
  return r_float() < ref;
}

DeliverJob *h_deliver_job(House *h, GameScene *gs) {
  if (h->resources_maximum.clicks <= 0)
    return NULL;

  bool need_water = h->resources_maximum.water - h->resources.water >= 1.0f;
  bool need_food = h->resources_maximum.food - h->resources.food >= 1.0f;
  bool food_is_more_urgent = need_water && need_food && h->resources.water > h->resources.food;
  if (!food_is_more_urgent && need_water && (gs->resource_pool.water - gs->resource_pool_claimed.water > 0)) {
    Recti marketplace = find_resource_building_rect(gs, h->display.location, R_Water);
    if (marketplace.w > 0) {
      gs->resource_pool_claimed.water++;
      h->resources_maximum.clicks--;
      return deliver_job(marketplace, h->display.location, MI_Water, wl_color(), h, (CollectDoneCB)h_pay_water,
                         (DeliverDoneCB)h_get_water_done);
    }
  } else if (need_food && (gs->resource_pool.food - gs->resource_pool_claimed.food > 0)) {
    Recti marketplace = find_resource_building_rect(gs, h->display.location, R_Food);
    if (marketplace.w > 0) {
      gs->resource_pool_claimed.food++;
      h->resources_maximum.clicks--;
      return deliver_job(marketplace, h->display.location, MI_Food, fa_color(), h, (CollectDoneCB)h_pay_food,
                         (DeliverDoneCB)h_get_food_done);
    }
  }
  return NULL;
}

bool w_deliver(Wearisome *w, GameScene *gs, DeliverJob *job);

bool w_check_what_to_do_next(Wearisome *w, GameScene *gs) {
  WearisomeState old_state = w->state;
  DeliverJob *job = NULL;

  if (!l_movableP(gs->level, l_to_point(w->destination)))
    return NULL;

  if (gs->daytime > 0.75f && w_move_to(w, gs, w->home->display.location))
    w->state = W_MovingHome;

  else if (w->needs.sleep < 0.25f && w_move_to(w, gs, w->home->display.location))
    w->state = W_MovingHome;

  else if (((w->needs.water < 0.25f && w->home->resources.water > 0.0f) ||
            (w->needs.food < 0.25f && w->home->resources.food > 0.0f)) &&
           w_move_to(w, gs, w->home->display.location))
    w->state = W_MovingHome;

  else if ((job = h_deliver_job(w->home, gs))) {
    w_deliver(w, gs, job);

  } else if (w_want_entertainment(w)) {
    TileContent *building = find_resource_building(gs, w_current_rect(w), R_Entertainment);
    if (building)
      tc_claim(building, gs, w, R_Entertainment);

  } else if (w_want_to_work(w, gs)) {
    TileContent *building = find_resource_building(gs, w_current_rect(w), R_Work);
    if (building)
      tc_claim(building, gs, w, R_Work);
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
      w->state = w->state == W_DeliverCollect ? W_DeliverWait : W_WorkDeliverWait;
      w->current_building = w->deliver_job->from;
    }
  }
}

void w_u_deliver_wait(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (!w->deliver_job)
    w->state = W_Waiting;
  else if (w->wait_time < 0.0f) {
    if (dj_on_collect(w->deliver_job, gs) && w_move_to(w, gs, w->deliver_job->to)) {
      w->state = w->state == W_DeliverWait ? W_Deliver : W_WorkDeliver;
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
      w->current_building = w->deliver_job->to;
      dj_on_delivered(w->deliver_job, gs);
      w->deliver_job = NULL;
      if (w->state == W_WorkDeliver) {
        w->wait_time = tc_start(l_contentP(gs->level, l_to_point(w->destination)), gs, R_Work);
        w->state = W_Working;
      } else
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
      w->wait_time = tc_start(l_contentP(gs->level, l_to_point(w->destination)), gs,
                              w->state == W_MoveToEntertainment ? R_Entertainment : R_Work);
      if (w->state == W_MoveToEntertainment) {
        h_earn_click(w->home, -1);
        gs->clicks++;
      }
      w->state = (w->state == W_MoveToWork) ? W_Working : W_GetEntertained;

      // w->current_rect = w->deliver_job->from;
    }
  }
}

void w_u_queue_move(Wearisome *w, GameScene *gs, float dt) {
  (void)gs;

  w->position = v_lerp_about(w->position, w->destination, dt * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else if (!qi_on_done(&w->queue, w, gs)) {
      qi_clear(&w->queue);
      w->state = W_Waiting;
    }
  }
}

void w_u_queue_wait(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (w->wait_time < 0.0f) {
    if (!qi_on_done(&w->queue, w, gs)) {
      qi_clear(&w->queue);
      w->state = W_Waiting;
    }
  }
}

void w_u_working(Wearisome *w, GameScene *gs, float dt) {
  (void)gs;

  w->wait_time -= dt;
  if (w->wait_time < 0.0f) {
    tc_done(l_contentP(gs->level, l_to_point(w->destination)), gs, w->state == W_Working ? R_Work : R_Entertainment);
    if (w->state == W_Working)
      h_earn_click(w->home, 1);
    w_wander_to_random_near_path(w, gs);
  }
}

static Needs one_factor = {.food = 1.0f, .water = 1.0f, .sleep = 1.0f};
static Needs working_factor = {.food = 1.3f, .water = 1.15f, .sleep = 1.4f};
static Needs entertainment_factor = {.food = 0.2f, .water = 0.2f, .sleep = 1.0f};
void w_update(Wearisome *w, GameScene *gs, Game *g, float dt) {
  (void)g;

  if (w_dead(w))
    return;

  const Needs *factor =
      w->state == W_Working ? &working_factor : (w->state == W_GetEntertained ? &entertainment_factor : &one_factor);
  w->needs.water = f_max(0.0f, w->needs.water - gs->daytime_step * w->need_consumption.water * factor->water);
  w->needs.food = f_max(0.0f, w->needs.food - gs->daytime_step * w->need_consumption.food * factor->food);
  w->needs.sleep = f_max(0.0f, w->needs.sleep - gs->daytime_step * w->need_consumption.sleep * factor->sleep);
  if (w->needs.water < 0.1f || w->needs.food < 0.1f || w->needs.sleep < 0.1f)
    w->health -= 2.0f * gs->daytime_step;
  else
    w->health = f_min(1.0f, w->health + gs->daytime_step / 8.0f);

  w->home->wearisome_at_home = w->state == W_AtHome;
  if ((w->home->wearisome_dead = w->health <= 0.0f))
    return;

  Point p = l_to_point(w->destination);
  if (!ri_contains(w->current_building, p.x, p.y))
    w->current_building = w_current_rect(w);

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
  case W_WorkDeliverCollect:
    w_u_deliver_collect(w, gs, dt);
    break;
  case W_DeliverWait:
  case W_WorkDeliverWait:
    w_u_deliver_wait(w, gs, dt);
    break;
  case W_Deliver:
  case W_WorkDeliver:
    w_u_deliver(w, gs, dt);
    break;

  case W_MoveToWork:
  case W_MoveToEntertainment:
    w_u_move_to_work(w, gs, dt);
    break;

  case W_QueueMove:
    w_u_queue_move(w, gs, dt);
    break;
  case W_QueueWait:
    w_u_queue_wait(w, gs, dt);
    break;

  case W_Working:
  case W_GetEntertained:
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

  if (w->state == W_Working || w->state == W_GetEntertained || w->state == W_AtHome)
    return;
  Vec2 p = v_add(w->position, (Vec2){0, 2});
  g_color(g, w_dead(w) ? rgb(0, 0, 0) : warn(w->health));
  const int o = (size_t)w / 17;
  if (w->state == W_QueueWait || w->state == W_Waiting)
    g_object(g, g_animation_buffer(g), Img_wearisome, w_dead(w) ? 0 : 1, p);
  else
    g_object(g, g_animation_buffer(g), Img_wearisome, w_dead(w) ? 0 : (o + g_frame(g)) % 4, p);

  if (w->state == W_Deliver || w->state == W_WorkDeliver) {
    g_color(g, white());
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){4, 3}), 0.5f);
    if (w->deliver_job) {
      g_color(g, w->deliver_job->color);
      g_objectS(g, g_animation_buffer(g), Img_menubar, w->deliver_job->icon, v_add(p, (Vec2){4, 3}), 0.5f);
    }
  }

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

bool w_is_home(Wearisome *w) { return w->state == W_AtHome; }
bool w_is_free(Wearisome *w) { return w->state == W_Wandering || w->state == W_Waiting; }

bool w_deliver(Wearisome *w, GameScene *gs, DeliverJob *job) {
  if (w_move_to(w, gs, job->from)) {
    w->deliver_job = job;
    w->state = W_DeliverCollect;
    return true;
  }
  return false;
}
bool w_deliver_work(Wearisome *w, GameScene *gs, DeliverJob *job) {
  if (w_move_to(w, gs, job->from)) {
    w->deliver_job = job;
    w->state = W_WorkDeliverCollect;
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
  Vec2 pos = l_to_vec(home->display.location.x, home->display.location.y);
  Wearisome *w = g_malloc(g, sizeof(Wearisome));
  *w = (Wearisome){
      .home = home,
      .current_building = home->display.location,
      .position = pos,
      .destination = pos,
      .path = NULL,
      .needs = {.food = 1.0f, .water = 1.0f, .sleep = 1.0f},
      .need_consumption = {.food = r_float_r(0.25f, 0.35f),
                           .water = r_float_r(0.35f, 0.45f),
                           .sleep = r_float_r(0.5f, 0.7f)},
      .health = 1.0f,
      .deliver_job = NULL,
      .state = W_AtHome,
      .speed = r_float_r(60.0f, 75.0f),
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