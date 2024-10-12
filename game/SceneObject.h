#ifndef SCENEOBJECT
#define SCENEOBJECT

#include "Game.h"
#include <stdbool.h>

typedef struct SceneObject SceneObject;

typedef struct SceneObjectTable {
  bool (*dead)(SceneObject *);
  void (*update)(SceneObject *, Game *);
  void (*draw)(SceneObject *, Game *);
} SceneObjectTable;

typedef struct SceneObject {
  void *context;
  SceneObjectTable *table;
} SceneObject;

static inline bool SceneObject_dead(SceneObject *so) { return so->table->dead && so->table->dead(so->context); }
static inline void SceneObject_update(SceneObject *so, Game *g) {
  if (so->table->update)
    so->table->update(so->context, g);
}
static inline void SceneObject_draw(SceneObject *so, Game *g) {
  if (so->table->draw)
    so->table->draw(so->context, g);
}

#endif