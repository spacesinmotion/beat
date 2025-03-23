#ifndef CONNECTION_H
#define CONNECTION_H

#include "extern/cjsonh/cjsonh.h"
#include "game/Game.h"
#include "game/GameScene.h"
#include "game/assets.h"
#include "math/Vec2.h"

typedef enum ConnectionState { CS_Defining, CS_Running, CS_Dead } ConnectionState;
typedef struct Connection {
  int id;
  Vec2 start, stop;
  ConnectionState state;
} Connection;

bool co_dead(Connection *co) { return co->state == CS_Dead; }

void co_update(Connection *co, GameScene *gs, Game *g, float dt) {
  (void)g;
  (void)gs;
  (void)co;
  (void)dt;
}

float co_render_order(Connection *co) { return co->start.y + 2000; }

void co_draw(Connection *co, GameScene *gs, Game *g) {
  (void)gs;

  g_color(g, rgba(146, 182, 189, 100));
  const float o = (-0.5f + fmod(g_time(g), 1.0)) / 30.0;
  for (int i = 1; i < 30; ++i) {
    const float t = (float)i / 30.0f + o;
    Vec2 l = v_lerp(co->start, co->stop, t);
    g_objectS(g, g_animation_buffer(g), Img_wearisome, 12, l, 0.1f + 0.15 * sin(t * M_PI));
  }
  g_color(g, rgba(146, 182, 189, 200));
  g_object(g, g_animation_buffer(g), Img_connections, 0, co->start);
}

void co_to_json(CJHObject *o, Connection *co) {
  cjh_o_add_number(o, "id", co->id);
  cjh_o_add_array(o, "start", (CJHWriteArrayCB)v_to_json, &co->start);
  cjh_o_add_array(o, "stop", (CJHWriteArrayCB)v_to_json, &co->stop);
  const char *state_test = "Defining";
  if (co->state == CS_Running)
    state_test = "Running";
  else if (co->state == CS_Dead)
    state_test = "Dead";
  cjh_o_add_string(o, "state", state_test);
}

void co_from_json(CJHObjectR *o, const char *key, Connection *co) {

  if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));

  else if (streq(key, "start")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)v_from_json, &co->start);
    indent -= 2;
  } else if (streq(key, "stop")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)v_from_json, &co->stop);
    indent -= 2;
  } else if (streq(key, "state")) {
    StrView s = cjh_o_read_string(o);
    printf("%.*s%s: %.*s\n", indent, space, key, s.len, s.s);
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

static SceneObjectTable Connection_table = {
    .type = "Connection",
    .dead = (SceneObjectDeadCB)co_dead,
    .render_order = (SceneObjectRenderOrderCB)co_render_order,
    .update = (SceneObjectUpdateCB)co_update,
    .draw = (SceneObjectDrawCB)co_draw,
    .save = (SceneObjectSaveCB)co_to_json,
};

void co_to_json_ref(CJHObject *o, Connection *co) { cjh_o_add_number(o, Connection_table.type, co ? co->id : 0); }

Connection *Connection_init(GameScene *gs, Vec2 s, Vec2 e) {
  Connection *h = g_malloc(sizeof(Connection));
  *h = (Connection){
      .id = unique_id(h),
      .start = s,
      .stop = e,
      .state = CS_Defining,
  };

  gs_add_object(gs, (SceneObject){h, &Connection_table});

  return h;
}
#endif