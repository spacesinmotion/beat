#ifndef SELECTABLE_H
#define SELECTABLE_H

#include <stdbool.h>

typedef struct Game Game;
typedef struct Selectable Selectable;

typedef bool (*SelectableMouseClick)(Selectable *, Game *);

typedef struct SelectableTable {
  SelectableMouseClick click;
} SelectableTable;

typedef struct Selectable {
  void *context;
  const SelectableTable *table;
} Selectable;

static inline bool sl_valid(const Selectable *sl) { return sl->table; }

static inline bool sl_eq(const Selectable *sl1, const Selectable *sl2) {
  return sl1 && sl2 && sl1->context == sl2->context;
}

static inline void sl_click(Selectable *sl, Game *g) {
  if (sl->table && sl->table->click)
    sl->table->click(sl->context, g);
}

#endif