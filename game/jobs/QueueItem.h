#ifndef QUEUEITEM_H
#define QUEUEITEM_H

#include "engine/extern/cjsonh/cjsonh.h"
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

void qi_to_json(CJHObject *o, QueueItem *qi) {
  cjh_o_add_number(o, "id", qi->id);
  cjh_o_add_string(o, "cb", qi->cb_name);
}

void qi_from_json(CJHObjectR *o, const char *key, QueueItem *qi) {
  (void)qi;

  if (streq(key, "id"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "cb")) {
    StrView s = cjh_o_read_string(o);
    printf("%.*s%s: %.*s\n", indent, space, key, s.len, s.s);
  }

  else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

#endif