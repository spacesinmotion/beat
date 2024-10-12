#ifndef SCENEOBJECT
#define SCENEOBJECT

#include "Game.h"
#include <stdbool.h>

typedef struct SceneObject SceneObject;

typedef bool (*SceneObjectDeadCB)(SceneObject *);
typedef bool (*SceneObjectUpdateCB)(SceneObject *, Game *, float);
typedef bool (*SceneObjectDrawCB)(SceneObject *, Game *);
typedef struct SceneObjectTable {
  SceneObjectDeadCB dead;
  SceneObjectUpdateCB update;
  SceneObjectDrawCB draw;
} SceneObjectTable;

typedef struct SceneObject {
  void *context;
  SceneObjectTable *table;
} SceneObject;

static inline bool SceneObject_dead(SceneObject *so) { return so->table->dead && so->table->dead(so->context); }
static inline void SceneObject_update(SceneObject *so, Game *g, float dt) {
  if (so->table->update)
    so->table->update(so->context, g, dt);
}
static inline void SceneObject_draw(SceneObject *so, Game *g) {
  if (so->table->draw)
    so->table->draw(so->context, g);
}

#endif