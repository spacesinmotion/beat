#ifndef BOARD_H
#define BOARD_H

#include "engine/math/Rect.h"
#include "game/ObjectType.h"
#include "game/PointOverview.h"
#include <stdbool.h>

typedef struct Board {
  ObjectType grid[7][7];
  bool visited[7][7];
  bool allowed_to_pick[7][7];
} Board;

static inline void bd_reset(Board *bd) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j) {
      bd->grid[i][j] = So_Empty;
      bd->visited[i][j] = false;
      bd->allowed_to_pick[i][j] = false;
    }
  bd->grid[0][0] = bd->grid[2][0] = bd->grid[4][0] = bd->grid[6][0] = So_None;
  bd->grid[3][3] = So_Water;
  bd->grid[0][1] = bd->grid[6][6] = So_House;
  bd->grid[0][6] = bd->grid[6][1] = So_Trees;
}

static inline ObjectType bd_get_grid(const Board *bd, Sizei gp) {
  if (gp.w >= 0 && gp.w < 7 && gp.h >= 0 && gp.h < 7)
    return bd->grid[gp.w][gp.h];
  return So_None;
}

static inline void bd_set_grid(Board *bd, Sizei gp, ObjectType value) {
  if (gp.w >= 0 && gp.w < 7 && gp.h >= 0 && gp.h < 7)
    bd->grid[gp.w][gp.h] = value;
}

static inline bool bd_grid_valid(const Board *bd, Sizei gp) {
  return gp.w >= 0 && gp.w < 7 && gp.h >= 0 && gp.h < 7 && bd_get_grid(bd, gp) != So_None;
}

static inline bool bd_grid_empty(const Board *bd, Sizei gp) { return bd_get_grid(bd, gp) == So_Empty; }

static inline void bd_clear_visited(Board *bd) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      bd->visited[i][j] = false;
}

static inline int bd_count_group_at(Board *bd, Sizei gp, ObjectType t) {
  if (!bd_grid_valid(bd, gp) || bd->visited[gp.w][gp.h] || t != bd_get_grid(bd, gp))
    return 0;

  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  int count = 1;
  bd->visited[gp.w][gp.h] = true;
  for (int i = 0; i < 6; ++i)
    count += bd_count_group_at(bd, n[i], t);
  return count;
}

static inline void bd_count_border_hits(Board *bd, Sizei gp, ObjectType t, bool *sides_hit) {
  if (gp.w < 0)
    sides_hit[0] = true;
  if (gp.w > 6)
    sides_hit[1] = true;
  if (gp.h > 6)
    sides_hit[3] = true;
  if ((gp.w & 1) == 0 && gp.h < 1)
    sides_hit[2] = true;
  if ((gp.w & 1) == 1 && gp.h < 0)
    sides_hit[2] = true;

  if (!bd_grid_valid(bd, gp) || bd->visited[gp.w][gp.h] || t != bd_get_grid(bd, gp))
    return;

  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  bd->visited[gp.w][gp.h] = true;
  for (int i = 0; i < 6; ++i)
    bd_count_border_hits(bd, n[i], t, sides_hit);
}

static inline void bd_set_allowed_to_pick(Board *bd, Sizei gp, bool value) {
  if (bd_grid_valid(bd, gp))
    bd->allowed_to_pick[gp.w][gp.h] = value;
}

static inline bool bd_allowed_to_pick(const Board *bd, Sizei gp) {
  return bd_grid_valid(bd, gp) && bd->allowed_to_pick[gp.w][gp.h];
}

static inline void bd_clear_allowed_to_pick(Board *bd) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      bd->allowed_to_pick[i][j] = false;
}

static inline void bd_count_points(Board *bd, PointOverview *po) {
  bd_clear_visited(bd);
  po->house.g1 = po->house.g2 = po->house.points = 0;
  po->trees.g1 = po->trees.g2 = po->trees.points = 0;
  po->animals.g1 = po->animals.g2 = po->animals.points = 0;
  po->flowers.g1 = po->flowers.g2 = po->flowers.points = 0;

  for (int i = 0; i < 7; ++i) {
    for (int j = 0; j < 7; ++j) {
      if (bd->visited[i][j])
        continue;

      const ObjectType t = bd_get_grid(bd, (Sizei){i, j});
      if (t == So_Empty || t == So_None || t == So_Water) {
        bd->visited[i][j] = true;
        continue;
      }

      const int c = bd_count_group_at(bd, (Sizei){i, j}, t);
      if (t == So_House) {
        po_group_counter_add_group(&po->house, c);
      } else if (t == So_Trees) {
        po_group_counter_add_group(&po->trees, c);
      } else if (t == So_Animals) {
        po_group_counter_add_group(&po->animals, c);
      } else if (t == So_Flowers) {
        po_group_counter_add_group(&po->flowers, c);
      }
    }
  }

  bd_clear_visited(bd);

  bool sides_hit[4] = {false, false, false, false};
  bd_count_border_hits(bd, (Sizei){3, 3}, So_Water, sides_hit);

  po->water_count = 0;
  for (int i = 0; i < 4; ++i)
    if (sides_hit[i])
      po->water_count++;

  po->points = po->house.points + po->trees.points + po->animals.points + po->flowers.points + po->water_count * 15;
}

static inline float bd_fill_ratio(const Board *bd) {
  int filled = 0;
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      if (bd->grid[i][j] != So_None && bd->grid[i][j] != So_Empty)
        filled++;
  return (float)filled / (49.0f - 4.0f);
}

#endif