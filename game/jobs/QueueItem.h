#ifndef QUEUEITEM_H
#define QUEUEITEM_H

#include "extern/cjsonh/cjsonh.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct GameScene GameScene;
typedef struct Wearisome Wearisome;

typedef bool (*QueueItemDoneCB)(void *, Wearisome *w, GameScene *);
typedef struct QueueItem {
  void *context;
  QueueItemDoneCB on_done;
  const char *cb_name;
} QueueItem;

#define QI(c, cb)                                                                                                      \
  (QueueItem) { (c), cb, #cb }

static inline bool qi_is_set(QueueItem *qi) { return qi->on_done != NULL; }

// void qi_set(QueueItem *qi, QueueItemDoneCB on_done, void *context) { *qi = (QueueItem){context, on_done}; }
void qi_clear(QueueItem *qi) { *qi = (QueueItem){NULL, NULL, NULL}; }

bool qi_on_done(QueueItem *qi, Wearisome *w, GameScene *gs) { return qi->on_done && qi->on_done(qi->context, w, gs); }

void qi_to_json(CJHObject *o, QueueItem *qi) { cjh_o_add_string(o, "cb", qi->cb_name); }

#endif