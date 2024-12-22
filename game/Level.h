#ifndef LEVEL_H
#define LEVEL_H

#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define LEVEL_WIDTH 48
#define LEVEL_HEIGHT 32

typedef enum TileType {
  T_None = 0,
  T_CityCenter,
  T_House,

  T_Path,
  T_PathStartEnd,

  T_Movable = 1 << 7,
} TileType;

typedef struct Level {
  uint8_t tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} Level;

void Level_init(Level *level) { memset(level->tiles, 0, sizeof(level->tiles)); }

bool Level_valid(Level *level, int x, int y) {
  (void)level;
  return x >= 0 && x < LEVEL_WIDTH && y >= 0 && y < LEVEL_HEIGHT;
}

bool Level_validP(Level *level, Point p) { return Level_valid(level, p.x, p.y); }
bool Level_validR(Level *level, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (!Level_valid(level, i, j))
        return false;
  return true;
}

bool Level_movable(Level *level, int x, int y) {
  return Level_valid(level, x, y) && ((level->tiles[x][y] & T_Movable) == T_Movable);
}
void Level_set_movable(Level *level, int x, int y, bool movable) {
  if (Level_valid(level, x, y)) {
    if (movable)
      level->tiles[x][y] |= T_Movable;
    else
      level->tiles[x][y] &= ~T_Movable;
  }
}

TileType Level_tile(Level *level, int x, int y) {
  return Level_valid(level, x, y) ? (TileType)(level->tiles[x][y] & ~T_Movable) : T_None;
}
void Level_set_tile(Level *level, int x, int y, TileType tile) {
  if (Level_valid(level, x, y))
    level->tiles[x][y] = Level_movable(level, x, y) ? tile | T_Movable : tile;
}

void Level_clear_paths(Level *level) {
  for (int i = 0; i < LEVEL_WIDTH; ++i) {
    for (int j = 0; j < LEVEL_HEIGHT; ++j) {
      TileType t = Level_tile(level, i, j);
      if (t == T_Path || t == T_PathStartEnd)
        Level_set_tile(level, i, j, T_None);
    }
  }
}

Vec2 Level_to_vec(int i, int j) { return (Vec2){i * 16.0f, j * 16.0f}; }

typedef struct {
  Point points[LEVEL_WIDTH * LEVEL_HEIGHT];
  int front, rear;
} Queue;

void init_queue(Queue *q) { q->front = q->rear = 0; }

bool is_queue_empty(Queue *q) { return q->front == q->rear; }

void enqueue(Queue *q, Point p) { q->points[q->rear++] = p; }

Point dequeue(Queue *q) { return q->points[q->front++]; }

typedef struct SearchHandle SearchHandle;
typedef bool (*CanMoveCB)(SearchHandle *, int x, int y);
typedef bool (*GoalReachedCB)(SearchHandle *, int x, int y);
typedef void (*PathCB)(SearchHandle *, int x, int y);
typedef struct SearchHandle {
  void *context;
  CanMoveCB can_move;
  GoalReachedCB goal_reached;
  PathCB path_callback;
} SearchHandle;

bool bfs(Level *level, int start_x, int start_y, SearchHandle handle) {
  (void)level;

  Point predecessor[LEVEL_WIDTH][LEVEL_HEIGHT];
  memset(predecessor, -1, sizeof(predecessor));

  Queue q;
  init_queue(&q);
  enqueue(&q, (Point){start_x, start_y});
  predecessor[start_x][start_y] = (Point){start_x, start_y}; // self

  int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

  while (!is_queue_empty(&q)) {
    Point p = dequeue(&q);

    if (handle.goal_reached(handle.context, p.x, p.y)) {
      Point c = p;
      while (predecessor[c.x][c.y].x != c.x || predecessor[c.x][c.y].y != c.y) {
        handle.path_callback(handle.context, c.x, c.y);
        c = predecessor[c.x][c.y];
      }
      return true;
    }

    for (int i = 0; i < 4; ++i) {
      int nx = p.x + directions[i][0];
      int ny = p.y + directions[i][1];

      if (nx < 0 || nx >= LEVEL_WIDTH || ny < 0 || ny >= LEVEL_HEIGHT)
        continue;

      if (predecessor[nx][ny].x < 0 && handle.can_move(handle.context, nx, ny)) {
        predecessor[nx][ny] = (Point){p.x, p.y};
        enqueue(&q, (Point){nx, ny});
      }
    }
  }

  return false;
}

#endif // LEVEL_H
