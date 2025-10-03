#ifndef SCENEOBJECT
#define SCENEOBJECT

#include "extern/cjsonh/cjsonh.h"
#include <float.h>
#include <stdbool.h>

typedef struct SceneObject SceneObject;
typedef struct GameScene GameScene;
typedef struct Game Game;
typedef struct CJHObject CJHObject;

typedef bool (*SceneObjectDieCB)(SceneObject *, Game *g);
typedef float (*SceneObjectRenderOrderCB)(const SceneObject *);
typedef void (*SceneObjectUpdateCB)(SceneObject *, GameScene *, Game *, float);
typedef void (*SceneObjectDrawCB)(const SceneObject *, GameScene *gs, Game *);
typedef void (*SceneObjectSaveCB)(CJHObject *o, const SceneObject *);

typedef struct SceneObjectTable {
  const char *type;
  SceneObjectDieCB die;
  SceneObjectRenderOrderCB render_order;
  SceneObjectUpdateCB update;
  SceneObjectDrawCB draw;
  SceneObjectSaveCB save;
} SceneObjectTable;

typedef struct SceneObject {
  void *context;
  const SceneObjectTable *table;
} SceneObject;

static inline bool so_eq(const SceneObject *so1, const SceneObject *so2) {
  return so1 && so2 && so1->context == so2->context;
}

static inline bool so_die(SceneObject *so, Game *g) {
  return !so->context || (so->table->die && so->table->die(so->context, g));
}

static inline float so_render_order(const SceneObject *so) {
  return so->table->render_order ? so->table->render_order(so->context) : FLT_MAX;
}

static inline void so_update(SceneObject *so, GameScene *gs, Game *g, float dt) {
  if (so->context && so->table->update)
    so->table->update(so->context, gs, g, dt);
}

static inline void so_draw(const SceneObject *so, GameScene *gs, Game *g) {
  if (so->context && so->table->draw)
    so->table->draw(so->context, gs, g);
}

static inline bool so_can_be_stored(const SceneObject *so) { return so->table->save && so->context; }
static inline void so_to_json(CJHObject *o, SceneObject *so) {
  if (so->table->save && so->context)
    cjh_o_add_object(o, so->table->type, (CJHWriteObjectCB)so->table->save, so->context);
  else
    cjh_o_add_null(o, so->table->type);
}

static inline int unique_id(const void *so) {
  (void)so;
  static int id = 1;
  return id++;
}

#endif