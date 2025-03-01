#ifndef CONNECTION_H
#define CONNECTION_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/assets.h"
#include "math/Vec2.h"

typedef enum ConnectionState { CS_Defining, CS_Running, CS_Dead } ConnectionState;
typedef struct Connection {
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

static SceneObjectTable Connection_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)co_dead,
    .render_order = (SceneObjectRenderOrderCB)co_render_order,
    .update = (SceneObjectUpdateCB)co_update,
    .draw = (SceneObjectDrawCB)co_draw,
};

Connection *Connection_init(GameScene *gs, Vec2 s, Vec2 e) {
  Connection *h = g_malloc(sizeof(Connection));
  *h = (Connection){
      .start = s,
      .stop = e,
      .state = CS_Defining,
  };

  gs_add_object(gs, (SceneObject){h, &Connection_table});

  return h;
}
#endif