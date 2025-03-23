#ifndef WEARISOME
#define WEARISOME

#include "extern/cjsonh/cjsonh.h"
#include "game/Game.h"
#include "game/House.h"
#include "game/assets.h"
#include "game/effects/Dust.h"
#include "game/jobs/QueueItem.h"
#include "game/search/RectSearch.h"
#include "game/search/ResourceProviderSearch.h"
#include "game/search/StreetSearch.h"
#include "math/random.h"

#include <assert.h>

typedef enum WearisomeState {
  W_None = 0,
  W_AtHome,
  W_MovingHome,
  W_Waiting,
  W_Wandering,

  W_QueueMove,
  W_QueueWait,
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
  case W_QueueMove:
    return "queue job move";
  case W_QueueWait:
    return "queue job wait";
  }
  assert(false);
  return "<error>";
}

typedef enum WearisomeNeedMode { W_Normal, W_IsWorking, W_GetEntertainment } WearisomeNeedMode;

const char *WearisomeNeedMode_name(WearisomeNeedMode s) {
  switch (s) {
  case W_Normal:
    return "Normal";
  case W_IsWorking:
    return "IsWorking";
  case W_GetEntertainment:
    return "GetEntertainment";
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

  QueueItem queue, queue_follow_up;
  WearisomeState state;
  WearisomeNeedMode need_mode;

  MenuIcon deliver_icon;
  Color deliver_color;

  int need_dust_frame;
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
bool w_queue_move_to(Wearisome *w, GameScene *gs, Recti location, QueueItem qi) {
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
static inline bool w_leave_building(Wearisome *w, GameScene *gs, QueueItem qi) {
  w->path = find_street(gs, w->current_building);
  if (!w->path)
    return false;
  w->queue = qi;
  w->state = W_QueueMove;
  return true;
}

static inline void w_earn_clicks(Wearisome *w, int c) { h_earn_click(w->home, c); }
void w_deliver(Wearisome *w, MenuIcon mi, Color c) {
  w->deliver_icon = mi;
  w->deliver_color = c;
}
void w_deliver_clear(Wearisome *w) { w->deliver_icon = Nb_MI; }

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

bool w_check_what_to_do_next(Wearisome *w, GameScene *gs) {
  WearisomeState old_state = w->state;

  if (!l_movableP(gs->level, l_to_point(w->destination)))
    return false;

  if (gs->daytime > 0.75f && w_move_to(w, gs, w->home->display.location))
    w->state = W_MovingHome;

  else if (w->needs.sleep < 0.25f && w_move_to(w, gs, w->home->display.location))
    w->state = W_MovingHome;

  else if (((w->needs.water < 0.25f && w->home->resources.water > 0.0f) ||
            (w->needs.food < 0.25f && w->home->resources.food > 0.0f)) &&
           w_move_to(w, gs, w->home->display.location))
    w->state = W_MovingHome;

  else if (h_check_needs(w->home, gs, w)) {
    return true;

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

void w_u_queue_move(Wearisome *w, GameScene *gs, float dt) {
  (void)gs;

  w->position = v_lerp_about(w->position, w->destination, dt * w->speed);
  if (v_eq(w->position, w->destination)) {
    if (w->path) {
      w->destination = w->path->p;
      w->path = w->path->next;
    } else if (!qi_on_done(&w->queue, w, gs)) {
      qi_clear(&w->queue);
      qi_clear(&w->queue_follow_up);
      w->state = W_Waiting;
    }
  }
}

void w_u_queue_wait(Wearisome *w, GameScene *gs, float dt) {
  w->wait_time -= dt;
  if (w->wait_time < 0.0f) {
    if (!qi_on_done(&w->queue, w, gs)) {
      qi_clear(&w->queue);
      qi_clear(&w->queue_follow_up);
      w->state = W_Waiting;
    }
  }
}

static Needs one_factor = {.food = 1.0f, .water = 1.0f, .sleep = 1.0f};
static Needs working_factor = {.food = 1.3f, .water = 1.15f, .sleep = 1.4f};
static Needs entertainment_factor = {.food = 0.2f, .water = 0.2f, .sleep = 1.0f};
void w_update(Wearisome *w, GameScene *gs, Game *g, float dt) {
  (void)g;
  if (w_dead(w))
    return;

  gs->wearisome_count++;
  const Needs *factor = w->need_mode == W_IsWorking
                            ? &working_factor
                            : (w->need_mode == W_GetEntertainment ? &entertainment_factor : &one_factor);
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

  case W_QueueMove:
    w_u_queue_move(w, gs, dt);
    break;
  case W_QueueWait:
    w_u_queue_wait(w, gs, dt);
    break;
  }

  if (w->need_dust_frame == g_frame(g)) {
    Dust_init(g, gs, v_add(w->position, (Vec2){0, -2}));
    w->need_dust_frame = -1;
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

  if (w->need_mode == W_IsWorking || w->need_mode == W_GetEntertainment || w->state == W_AtHome)
    return;
  Vec2 p = v_add(w->position, (Vec2){0, 2});
  g_color(g, w_dead(w) ? rgb(0, 0, 0) : warn(w->health));
  const int o = (size_t)w % 17;
  if (w->state == W_QueueWait || w->state == W_Waiting)
    g_object(g, g_animation_buffer(g), Img_wearisome, w_dead(w) ? 0 : 1, p);
  else {
    const int f = g_frame(g);
    g_object(g, g_animation_buffer(g), Img_wearisome, w_dead(w) ? 0 : (o + f) % 4, p);
    if ((o + f) % 4 == 0)
      w->need_dust_frame = f + 1;
  }

  if (w->deliver_icon < Nb_MI) {
    g_color(g, white());
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, v_add(p, (Vec2){4, 3}), 0.5f);
    g_color(g, w->deliver_color);
    g_objectS(g, g_animation_buffer(g), Img_menubar, w->deliver_icon, v_add(p, (Vec2){4, 3}), 0.5f);
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

void w_needs_to_json(CJHObject *o, void *np) {
  Needs *n = (Needs *)np;
  cjh_o_add_number(o, "food", n->food);
  cjh_o_add_number(o, "water", n->water);
  cjh_o_add_number(o, "sleep", n->sleep);
}

void w_to_json(CJHObject *o, Wearisome *w) {
  cjh_o_add_object(o, "house", (CJHWriteObjectCB)h_to_json_ref, w->home);
  cjh_o_add_array(o, "current_building", (CJHWriteArrayCB)ri_to_json, &w->current_building);
  cjh_o_add_array(o, "position", (CJHWriteArrayCB)v_to_json, &w->position);
  cjh_o_add_array(o, "destination", (CJHWriteArrayCB)v_to_json, &w->destination);

  cjh_o_add_array(o, "path", (CJHWriteArrayCB)pp_to_json, w->path);
  cjh_o_add_number(o, "wait_time", w->wait_time);
  cjh_o_add_object(o, "needs", w_needs_to_json, &w->needs);
  cjh_o_add_object(o, "need_consumption", w_needs_to_json, &w->need_consumption);

  cjh_o_add_number(o, "health", w->health);
  cjh_o_add_number(o, "speed", w->speed);

  // ?
  // QueueItem queue, queue_follow_up;
  if (qi_is_set(&w->queue))
    cjh_o_add_object(o, "queue", (CJHWriteObjectCB)qi_to_json, &w->queue);
  if (qi_is_set(&w->queue_follow_up))
    cjh_o_add_object(o, "queue_follow_up", (CJHWriteObjectCB)qi_to_json, &w->queue_follow_up);

  cjh_o_add_string(o, "state", WearisomeState_name(w->state));
  cjh_o_add_string(o, "need_mode", WearisomeNeedMode_name(w->need_mode));
  cjh_o_add_number(o, "deliver_icon", w->deliver_icon);
  cjh_o_add_array(o, "deliver_color", (CJHWriteArrayCB)c_to_json, &w->deliver_color);
}

void w_needs_from_json(CJHObjectR *o, const char *key, Needs *np) {
  if (streq(key, "food"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "water"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "sleep"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

void w_from_json(CJHObjectR *o, const char *key, Wearisome *w) {

  if (streq(key, "health"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "speed"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "wait_time"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "deliver_icon"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "deliver_color")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)c_from_json, &w->needs);
    indent -= 2;

  } else if (streq(key, "state")) {
    StrView s = cjh_o_read_string(o);
    printf("%.*s%s: %.*s\n", indent, space, key, s.len, s.s);
  } else if (streq(key, "need_mode")) {
    StrView s = cjh_o_read_string(o);
    printf("%.*s%s: %.*s\n", indent, space, key, s.len, s.s);
  }

  else if (streq(key, "current_building")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)ri_from_json, &w->current_building);
    indent -= 2;
  } else if (streq(key, "position")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)v_from_json, &w->position);
    indent -= 2;
  } else if (streq(key, "destination")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)v_from_json, &w->destination);
    indent -= 2;
  } else if (streq(key, "path")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)pp_from_json, &w->path);
    indent -= 2;
  } else if (streq(key, "needs")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)w_needs_from_json, &w->needs);
    indent -= 2;
  } else if (streq(key, "need_consumption")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)w_needs_from_json, &w->need_consumption);
    indent -= 2;
  } else if (streq(key, "queue")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)qi_from_json, &w->queue);
    indent -= 2;
  } else if (streq(key, "queue_follow_up")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_object(o, (CJHReadObjectCB)qi_from_json, &w->queue_follow_up);
    indent -= 2;

  } else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

SceneObjectTable Wearisome_table = {
    .type = "Wearisome",
    .dead = (SceneObjectDeadCB)w_dead,
    .render_order = (SceneObjectRenderOrderCB)w_render_order,
    .update = (SceneObjectUpdateCB)w_update,
    .draw = (SceneObjectDrawCB)w_draw,
    .save = (SceneObjectSaveCB)w_to_json,
};

Wearisome *Wearisome_init(GameScene *gs, House *home) {
  Vec2 pos = l_to_vec(home->display.location.x, home->display.location.y);
  Wearisome *w = g_malloc(sizeof(Wearisome));
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
      .state = W_AtHome,
      .need_mode = W_Normal,
      .speed = r_float_r(60.0f, 75.0f),
      .deliver_icon = Nb_MI,
      .need_dust_frame = -1,
  };
  gs_add_object(gs, (SceneObject){.context = w, &Wearisome_table});
  return w;
}

House *Wearisome_House_init(Game *g, GameScene *gs, Point p) {
  House *h = House_init(g, gs, p);
  Wearisome_init(gs, h);
  return h;
}

#endif