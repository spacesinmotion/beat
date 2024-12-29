#ifndef SCENEOBJECT
#define SCENEOBJECT

#include "Game.h"
#include <float.h>
#include <stdbool.h>

typedef struct SceneObject SceneObject;
typedef struct GameScene GameScene;

typedef bool (*SceneObjectDeadCB)(const SceneObject *);
typedef float (*SceneObjectRenderOrderCB)(const SceneObject *);
typedef void (*SceneObjectUpdateCB)(SceneObject *, GameScene *, float);
typedef void (*SceneObjectDrawCB)(SceneObject *, Game *);

typedef struct SceneObjectTable {
  SceneObjectDeadCB dead;
  SceneObjectRenderOrderCB render_order;
  SceneObjectUpdateCB update;
  SceneObjectDrawCB draw;
} SceneObjectTable;

typedef struct SceneObject {
  void *context;
  const SceneObjectTable *table;
} SceneObject;

static inline bool SceneObject_eq(const SceneObject *so1, const SceneObject *so2) {
  return so1 && so2 && so1->context == so2->context;
}

static inline bool SceneObject_dead(const SceneObject *so) {
  return !so->context || (so->table->dead && so->table->dead(so->context));
}

static inline float SceneObject_render_order(const SceneObject *so) {
  return so->table->render_order ? so->table->render_order(so->context) : FLT_MAX;
}

static inline void SceneObject_update(SceneObject *so, GameScene *gs, float dt) {
  if (so->context && so->table->update)
    so->table->update(so->context, gs, dt);
}

static inline void SceneObject_draw(SceneObject *so, Game *g) {
  if (so->context && so->table->draw)
    so->table->draw(so->context, g);
}

#endif