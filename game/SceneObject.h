#ifndef SCENEOBJECT
#define SCENEOBJECT

#include "Game.h"
#include "math/Vec2.h"
#include <stdbool.h>

typedef struct SceneObject SceneObject;

typedef bool (*SceneObjectDeadCB)(SceneObject *);
typedef void (*SceneObjectUpdateCB)(SceneObject *, Game *, float);
typedef void (*SceneObjectDrawCB)(SceneObject *, Game *);
typedef bool (*SceneObjectMouseHitCB)(SceneObject *, Vec2);
typedef bool (*SceneObjectEnterCB)(SceneObject *, Game *);
typedef bool (*SceneObjectLeaveCB)(SceneObject *, Game *);
typedef struct SceneObjectTable {
  SceneObjectDeadCB dead;
  SceneObjectUpdateCB update;
  SceneObjectDrawCB draw;
  SceneObjectMouseHitCB mouse_hit;
  SceneObjectEnterCB enter;
  SceneObjectLeaveCB leave;
} SceneObjectTable;

typedef struct SceneObject {
  void *context;
  SceneObjectTable *table;
} SceneObject;

static inline bool SceneObject_dead(SceneObject *so) {
  return !so->context || (so->table->dead && so->table->dead(so->context));
}
static inline void SceneObject_update(SceneObject *so, Game *g, float dt) {
  if (so->context && so->table->update)
    so->table->update(so->context, g, dt);
}
static inline void SceneObject_draw(SceneObject *so, Game *g) {
  if (so->context && so->table->draw)
    so->table->draw(so->context, g);
}

static inline bool SceneObject_mouse_hit(SceneObject *so, Vec2 mp) {
  return so->context && so->table->mouse_hit && so->table->mouse_hit(so->context, mp);
}

static inline void SceneObject_enter(SceneObject *so, Game *g) {
  if (so->context && so->table->enter)
    so->table->enter(so->context, g);
}

static inline void SceneObject_leave(SceneObject *so, Game *g) {
  if (so->context && so->table->leave)
    so->table->leave(so->context, g);
}
#endif