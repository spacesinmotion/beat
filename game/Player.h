#ifndef PLAYER_H
#define PLAYER_H

#include "math/Rect.h"

typedef struct Stuff {
  int money, food, wood, iron;
} Stuff;

typedef struct Player {
  Stuff resources;
  Recti play_field;
} Player;

static inline bool pl_on_field(Player *player, Recti r) { return ri_containsR(player->play_field, r); }

#endif