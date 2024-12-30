
#ifndef TILECONTENT_H
#define TILECONTENT_H

#include "gc/gc.h"
#include <stdbool.h>

typedef struct TileContent TileContent;
typedef struct GameScene GameScene;

typedef bool (*HasWorkCB)(void *);
typedef void (*ClaimWorkCB)(void *);
typedef float (*StartWorkCB)(void *);
typedef void (*DoneWorkCB)(void *);
typedef void (*ClickCB)(const TileContent *, GameScene *gs);

typedef struct TileContentTable {
  HasWorkCB has_work;
  ClaimWorkCB claim_work;
  StartWorkCB start_work;
  DoneWorkCB done_work;
  ClickCB click;
} TileContentTable;
typedef struct TileContent {
  void *context;
  const TileContentTable *table;
} TileContent;

bool tc_has_work(const TileContent *tc) { return tc && tc->table->has_work && tc->table->has_work(tc->context); }
void tc_claim_work(const TileContent *tc) {
  if (tc && tc->table->claim_work)
    tc->table->claim_work(tc->context);
}
float tc_start_work(const TileContent *tc) {
  if (tc && tc->table->start_work)
    return tc->table->start_work(tc->context);
  return 1.0;
}
void tc_done_work(const TileContent *tc) {
  if (tc && tc->table->done_work)
    tc->table->done_work(tc->context);
}
void tc_click(const TileContent *tc, GameScene *gs) {
  if (tc && tc->table->click)
    tc->table->click(tc->context, gs);
}

TileContent *to_TileContent(void *d, const TileContentTable *t) {
  TileContent *tile_content = gc_malloc(&gc, sizeof(TileContent));
  *tile_content = (TileContent){d, t};
  return tile_content;
}

#endif