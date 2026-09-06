#ifndef QUEUEITEM_H
#define QUEUEITEM_H

#include <stdbool.h>
#include <stddef.h>

typedef struct GameScene GameScene;
typedef struct Wearisome Wearisome;

typedef bool (*QueueItemDoneCB)(void *, Wearisome *w, GameScene *);
typedef struct QueueItem {
  void *context;
  int id;
  QueueItemDoneCB on_done;
  const char *cb_name;
} QueueItem;

#define QI(c, cb)                                                                                                      \
  (QueueItem) { (c), (c)->id, cb, #cb }

static inline bool qi_is_set(QueueItem *qi) { return qi->on_done != NULL; }

// void qi_set(QueueItem *qi, QueueItemDoneCB on_done, void *context) { *qi = (QueueItem){context, on_done}; }
void qi_clear(QueueItem *qi) { *qi = (QueueItem){NULL, 0, NULL, NULL}; }

bool qi_on_done(QueueItem *qi, Wearisome *w, GameScene *gs) { return qi->on_done && qi->on_done(qi->context, w, gs); }

#endif