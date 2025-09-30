#ifndef SCENE_H
#define SCENE_H

#include "engine/math/Vec2.h"

typedef struct Game Game;
typedef struct CJHObject CJHObject;
typedef struct CJHObjectR CJHObjectR;

typedef void (*SceneUpdateCB)(void *, Game *, float);
typedef void (*SceneDrawCB)(void *, Game *);
typedef void (*SceneMouseMoveCB)(void *, Game *);
typedef void (*SceneMouseCB)(void *, Game *, int);
typedef void (*SceneKeyCB)(void *, Game *, int);

typedef void (*SceneSaveCB)(CJHObject *, void *);
typedef void (*SceneLoadCB)(CJHObjectR *, const char *, void *);

typedef struct SceneTable {
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

#endif