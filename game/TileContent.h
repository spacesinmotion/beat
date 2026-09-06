
#ifndef TILECONTENT_H
#define TILECONTENT_H

#include "engine/Game.h"
#include "engine/math/Rect.h"
#include <assert.h>
#include <stdbool.h>

typedef enum Resource {
  R_None = 0,

  R_Work,
  R_Entertainment,

  R_ManagerWork1,
  R_ManagerWork2,
  R_ManagerWork3,
  R_ManagerWork4,

  R_Water,
  R_Food,
  R_ConstructionMaterial,

  R_Deliver,
} Resource;

typedef struct TileContent TileContent;
typedef struct GameScene GameScene;
typedef struct Wearisome Wearisome;

typedef Recti (*LocationCb)(const void *);
typedef bool (*ProvidesCB)(void *, GameScene *gs, Resource r);
typedef void (*ClaimCB)(void *, GameScene *gs, Wearisome *w, Resource r);
typedef void (*TakeCB)(void *, GameScene *gs, Resource r, bool pay);
typedef void (*ClickCBx)(const void *, Point p, GameScene *gs);

typedef struct TileContentTable {
  LocationCb location;
  ProvidesCB provides;
  ClaimCB claim;
  TakeCB take;
  ClickCBx click;
} TileContentTable;
typedef struct TileContent {
  void *context;
  const TileContentTable *table;
} TileContent;

static inline Recti tc_location(const TileContent *tc) { return tc->table->location(tc->context); }
static inline bool tc_provides(const TileContent *tc, GameScene *gs, Resource r) {
  return tc && tc->table->provides && tc->table->provides(tc->context, gs, r);
}
static inline void tc_claim(const TileContent *tc, GameScene *gs, Wearisome *w, Resource r) {
  if (tc && tc->table->claim)
    tc->table->claim(tc->context, gs, w, r);
}
static inline void tc_take(const TileContent *tc, GameScene *gs, Resource r, bool pay) {
  if (tc && tc->table->take)
    tc->table->take(tc->context, gs, r, pay);
}

inline static bool tc_can_click(const TileContent *tc) { return (tc && tc->table->click); }
inline static void tc_click(const TileContent *tc, Point p, GameScene *gs) {
  if (tc && tc->table->click)
    tc->table->click(tc->context, p, gs);
}

static inline TileContent *to_TileContent(void *d, const TileContentTable *t) {
  assert(t->location);
  TileContent *tile_content = g_malloc(sizeof(TileContent));
  *tile_content = (TileContent){d, t};
  return tile_content;
}

#endif