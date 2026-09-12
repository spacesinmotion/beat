#ifndef LEVEL_H
#define LEVEL_H

#include "SokEngWrap/math/Rect.h"
#include "SokEngWrap/math/Vec2.h"
#include "game/TileContent.h"
#include <stdbool.h>
#include <string.h>

#define LEVEL_WIDTH 48
#define LEVEL_HEIGHT 32

typedef struct TileContent TileContent;
typedef struct Tile {
  bool movable;
  TileContent *content;
} Tile;
typedef struct Level {
  Tile tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} Level;

static inline void l_init(Level *level) { memset(level->tiles, 0, sizeof(level->tiles)); }

static inline bool l_valid(const Level *level, int x, int y) {
  (void)level;
  return x >= 0 && x < LEVEL_WIDTH && y >= 0 && y < LEVEL_HEIGHT;
}
static inline bool l_validP(Level *level, Point p) { return l_valid(level, p.x, p.y); }
static inline bool l_validR(Level *level, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (!l_valid(level, i, j))
        return false;
  return true;
}

bool l_free(Level *level, int x, int y) {
  return l_valid(level, x, y) && !level->tiles[x][y].movable && level->tiles[x][y].content == NULL;
}
bool l_freeP(Level *level, Point p) { return l_free(level, p.x, p.y); }
bool l_freeR(Level *level, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (!l_free(level, i, j))
        return false;
  return true;
}

static inline bool l_movable(Level *level, int x, int y) { return l_valid(level, x, y) && level->tiles[x][y].movable; }
static inline bool l_movableP(Level *level, Point p) { return l_movable(level, p.x, p.y); }
static inline void l_set_movable(Level *level, int x, int y, bool movable) {
  if (l_valid(level, x, y))
    level->tiles[x][y].movable = movable;
}

static inline void l_set_tile_content(Level *level, int x, int y, TileContent *c) {
  if (l_valid(level, x, y))
    level->tiles[x][y].content = c;
}
static inline void l_set_tile_contentR(Level *level, Recti r, TileContent *c) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      l_set_tile_content(level, i, j, c);
}

static inline TileContent *l_content(const Level *l, int x, int y) {
  return l_valid(l, x, y) ? l->tiles[x][y].content : NULL;
}
static inline TileContent *l_contentP(const Level *l, Point p) { return l_content(l, p.x, p.y); }

static const float F = 16.0f;
static inline float l_to_x(int i) { return i * F; }
static inline float l_to_y(int j) { return j * F; }
static inline Vec2 l_to_vec(int i, int j) { return (Vec2){i * F, j * F}; }
static inline Vec2 l_to_vecP(Point p) { return l_to_vec(p.x, p.y); }
static inline Rect l_to_vecR(Recti r) { return (Rect){l_to_vec(r.x, r.y), l_to_vec(r.w, r.h)}; }

static inline Point l_to_point(Vec2 v) { return (Point){v.x / F, v.y / F}; }

typedef struct Queue {
  Point points[LEVEL_WIDTH * LEVEL_HEIGHT];
  int front, rear;
} Queue;

static inline void q_init_queue(Queue *q) { q->front = q->rear = 0; }
static inline bool q_is_queue_empty(Queue *q) { return q->front == q->rear; }
static inline void q_enqueue(Queue *q, Point p) { q->points[q->rear++] = p; }
static inline Point q_dequeue(Queue *q) { return q->points[q->front++]; }

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

bool l_bright_first(Level *level, int start_x, int start_y, SearchHandle handle) {
  (void)level;

  Point predecessor[LEVEL_WIDTH][LEVEL_HEIGHT];
  memset(predecessor, -1, sizeof(predecessor));

  Queue q;
  q_init_queue(&q);
  q_enqueue(&q, (Point){start_x, start_y});
  predecessor[start_x][start_y] = (Point){start_x, start_y}; // self

  const int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

  while (!q_is_queue_empty(&q)) {
    Point p = q_dequeue(&q);

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
        q_enqueue(&q, (Point){nx, ny});
      }
    }
  }

  return false;
}

#endif // LEVEL_H
