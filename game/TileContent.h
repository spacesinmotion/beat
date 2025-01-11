
#ifndef TILECONTENT_H
#define TILECONTENT_H

#include "gc/gc.h"
#include <stdbool.h>

typedef enum Resource {
  R_None = 0,

  R_Work,
  R_Entertainment,

  R_Water,
  R_Food,
} Resource;

typedef struct TileContent TileContent;
typedef struct GameScene GameScene;

typedef bool (*ProvidesCB)(void *, GameScene *gs, Resource r);
typedef void (*ClaimCB)(void *, GameScene *gs, Resource r);
typedef float (*StartCB)(void *, GameScene *gs, Resource r);
typedef void (*DoneCB)(void *, GameScene *gs, Resource r);
typedef void (*ClickCB)(const TileContent *, GameScene *gs);

typedef struct TileContentTable {
  ProvidesCB provides;
  ClaimCB claim;
  StartCB start;
  DoneCB done;
  ClickCB click;
} TileContentTable;
typedef struct TileContent {
  void *context;
  const TileContentTable *table;
} TileContent;

bool tc_provides(const TileContent *tc, GameScene *gs, Resource r) {
  return tc && tc->table->provides && tc->table->provides(tc->context, gs, r);
}
void tc_claim(const TileContent *tc, GameScene *gs, Resource r) {
  if (tc && tc->table->claim)
    tc->table->claim(tc->context, gs, r);
}
float tc_start(const TileContent *tc, GameScene *gs, Resource r) {
  if (tc && tc->table->start)
    return tc->table->start(tc->context, gs, r);
  return 1.0;
}
void tc_done(const TileContent *tc, GameScene *gs, Resource r) {
  if (tc && tc->table->done)
    tc->table->done(tc->context, gs, r);
}
inline static bool tc_can_click(const TileContent *tc) { return (tc && tc->table->click); }
inline static void tc_click(const TileContent *tc, GameScene *gs) {
  if (tc && tc->table->click)
    tc->table->click(tc->context, gs);
}

TileContent *to_TileContent(void *d, const TileContentTable *t) {
  TileContent *tile_content = gc_malloc(&gc, sizeof(TileContent));
  *tile_content = (TileContent){d, t};
  return tile_content;
}

#endif