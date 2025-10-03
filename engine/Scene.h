#ifndef SCENE_H
#define SCENE_H

#include <stdlib.h>

typedef struct Game Game;
typedef struct CJHObject CJHObject;
typedef struct CJHObjectR CJHObjectR;

typedef void (*SceneInitCB)(void *, Game *);
typedef void (*SceneFreeCB)(void *, Game *);

typedef void (*SceneUpdateCB)(void *, Game *, float);
typedef void (*SceneDrawCB)(void *, Game *);
typedef void (*SceneMouseMoveCB)(void *, Game *);
typedef void (*SceneMouseCB)(void *, Game *, int);
typedef void (*SceneKeyCB)(void *, Game *, int);

typedef void (*SceneSaveCB)(CJHObject *, void *);
typedef void (*SceneLoadCB)(CJHObjectR *, const char *, void *);

typedef struct SceneTable {
  SceneInitCB init;
  SceneFreeCB free;

  SceneUpdateCB update;
  SceneDrawCB draw;
  SceneDrawCB draw_overlay;
  SceneMouseMoveCB mouse_move;
  SceneMouseCB mouse_down;
  SceneMouseCB mouse_up;
  SceneKeyCB key_down;
  SceneKeyCB key_up;

  SceneSaveCB save;
  SceneLoadCB load;
} SceneTable;
typedef struct Scene {
  void *context;
  const SceneTable *table;
} Scene;

static inline void sc_init(Scene *s, Game *g) {
  if (s->table && s->table->init)
    s->table->init(s->context, g);
}

static inline void sc_free(Scene *s, Game *g) {
  if (s->table && s->table->free)
    s->table->free(s->context, g);
  s->context = NULL;
  s->table = NULL;
}

#endif