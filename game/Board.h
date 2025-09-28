#ifndef BOARD_H
#define BOARD_H

#include "engine/math/Rect.h"
#include "game/ObjectType.h"
#include <stdbool.h>

typedef struct Board {
  ObjectType grid[7][7];
  bool visited[7][7];
  bool allowed_to_pick[7][7];
} Board;

static inline void board_init(Board *board) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j) {
      board->grid[i][j] = So_Empty;
      board->visited[i][j] = false;
      board->allowed_to_pick[i][j] = false;
    }
}

static inline ObjectType board_get_grid(const Board *board, Sizei gp) {
  if (gp.w >= 0 && gp.w < 7 && gp.h >= 0 && gp.h < 7)
    return board->grid[gp.w][gp.h];
  return So_None;
}

static inline void board_set_grid(Board *board, Sizei gp, ObjectType value) {
  if (gp.w >= 0 && gp.w < 7 && gp.h >= 0 && gp.h < 7)
    board->grid[gp.w][gp.h] = value;
}

static inline bool board_valid_grid(const Board *board, Sizei gp) {
  return gp.w >= 0 && gp.w < 7 && gp.h >= 0 && gp.h < 7 && board_get_grid(board, gp) != So_None;
}

static inline bool board_empty_grid(const Board *board, Sizei gp) {
  return board_valid_grid(board, gp) && board_get_grid(board, gp) == So_Empty;
}

static inline void board_clear_visited(Board *board) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      board->visited[i][j] = false;
}

static inline int board_count_group_at(Board *board, Sizei gp, ObjectType t) {
  if (!board_valid_grid(board, gp) || board->visited[gp.w][gp.h] || t != board_get_grid(board, gp))
    return 0;

  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  int count = 1;
  board->visited[gp.w][gp.h] = true;
  for (int i = 0; i < 6; ++i)
    count += board_count_group_at(board, n[i], t);
  return count;
}

static inline void board_count_side_hits(Board *board, Sizei gp, ObjectType t, bool *sides_hit) {
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

  if (!board_valid_grid(board, gp) || board->visited[gp.w][gp.h] || t != board_get_grid(board, gp))
    return;

  const Sizei n[6] = {{gp.w + 1, gp.h},
                      {gp.w - 1, gp.h},
                      {gp.w, gp.h + 1},
                      {gp.w, gp.h - 1},
                      {gp.w + 1, gp.h + (gp.w & 1 ? 1 : -1)},
                      {gp.w - 1, gp.h + (gp.w & 1 ? 1 : -1)}};
  board->visited[gp.w][gp.h] = true;
  for (int i = 0; i < 6; ++i)
    board_count_side_hits(board, n[i], t, sides_hit);
}

static inline void board_set_allowed_to_pick(Board *board, Sizei gp, bool value) {
  if (board_valid_grid(board, gp)) {
    board->allowed_to_pick[gp.w][gp.h] = value;
  }
}

static inline bool board_get_allowed_to_pick(const Board *board, Sizei gp) {
  if (board_valid_grid(board, gp)) {
    return board->allowed_to_pick[gp.w][gp.h];
  }
  return false;
}

static inline void board_clear_allowed_to_pick(Board *board) {
  for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
      board->allowed_to_pick[i][j] = false;
}

#endif