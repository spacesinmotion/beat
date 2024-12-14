#ifndef SCENEOBJECT
#define SCENEOBJECT

#include "Game.h"
#include "math/Circ.h"
#include <assert.h>
#include <stdbool.h>

typedef struct SceneObject SceneObject;
typedef struct GameScene GameScene;

typedef bool (*SceneObjectDeadCB)(const SceneObject *);
typedef Circle (*SceneObjectCircle)(const SceneObject *);
typedef void (*SceneObjectUpdateCB)(SceneObject *, Game *, GameScene *, float);
typedef void (*SceneObjectDrawCB)(SceneObject *, Game *);

typedef struct SceneObjectTable {
  SceneObjectDeadCB dead;
  SceneObjectCircle circle;
  SceneObjectUpdateCB update;
  SceneObjectDrawCB draw;
} SceneObjectTable;

typedef struct SceneObject {
  void *context;
  SceneObjectTable *table;
} SceneObject;

static inline bool SceneObject_eq(SceneObject *so1, SceneObject *so2) {
  return so1 && so2 && so1->context == so2->context;
}

static inline bool SceneObject_dead(SceneObject *so) {
  return !so->context || (so->table->dead && so->table->dead(so->context));
}

static inline Circle SceneObject_circle(SceneObject *so) {
  assert(so->context && so->table->circle);
  return so->table->circle(so->context);
}

static inline void SceneObject_update(SceneObject *so, Game *g, GameScene *gs, float dt) {
  if (so->context && so->table->update)
    so->table->update(so->context, g, gs, dt);
}

static inline void SceneObject_draw(SceneObject *so, Game *g) {
  if (so->context && so->table->draw)
    so->table->draw(so->context, g);
}

#endif