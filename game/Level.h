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
  T_Marketplace,
  T_House,
  T_Farm,

  T_Movable = 1 << 7,
} TileType;

typedef struct TileContent TileContent;
typedef struct GameScene GameScene;
typedef bool (*HasWorkCB)(void *);
typedef void (*ClaimWorkCB)(void *);
typedef float (*StartWorkCB)(void *);
typedef void (*DoneWorkCB)(void *);
typedef void (*ClickCB)(const TileContent *, GameScene *gs);
typedef struct TileContent {
  void *context;
  HasWorkCB has_work;
  ClaimWorkCB claim_work;
  StartWorkCB start_work;
  DoneWorkCB done_work;
  ClickCB click;
} TileContent;

typedef struct Tile {
  uint8_t val;
  TileContent *content;
} Tile;
typedef struct Level {
  Tile tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} Level;

void l_init(Level *level) { memset(level->tiles, 0, sizeof(level->tiles)); }

bool l_valid(const Level *level, int x, int y) {
  (void)level;
  return x >= 0 && x < LEVEL_WIDTH && y >= 0 && y < LEVEL_HEIGHT;
}
bool l_validP(Level *level, Point p) { return l_valid(level, p.x, p.y); }
bool l_validR(Level *level, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (!l_valid(level, i, j))
        return false;
  return true;
}

bool l_free(Level *level, int x, int y) { return l_valid(level, x, y) && level->tiles[x][y].val == T_None; }
bool l_freeP(Level *level, Point p) { return l_free(level, p.x, p.y); }
bool l_freeR(Level *level, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (!l_free(level, i, j))
        return false;
  return true;
}

bool l_movable(Level *level, int x, int y) {
  return l_valid(level, x, y) && ((level->tiles[x][y].val & T_Movable) == T_Movable);
}
void l_set_movable(Level *level, int x, int y, bool movable) {
  if (l_valid(level, x, y)) {
    if (movable)
      level->tiles[x][y].val |= T_Movable;
    else
      level->tiles[x][y].val &= ~T_Movable;
  }
}

TileType l_tile(Level *level, int x, int y) {
  return l_valid(level, x, y) ? (TileType)(level->tiles[x][y].val & ~T_Movable) : T_None;
}
void l_set_tile(Level *level, int x, int y, TileType tile) {
  if (l_valid(level, x, y))
    level->tiles[x][y].val = l_movable(level, x, y) ? tile | T_Movable : tile;
}
void l_set_tileR(Level *level, Recti r, TileType tile) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      l_set_tile(level, i, j, tile);
}

void l_set_tile_content(Level *level, int x, int y, TileContent *c) {
  if (l_valid(level, x, y))
    level->tiles[x][y].content = c;
}
void l_set_tile_contentR(Level *level, Recti r, TileContent *c) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      l_set_tile_content(level, i, j, c);
}

bool l_has_work(Level *l, int x, int y) {
  return l_valid(l, x, y) && l->tiles[x][y].content && l->tiles[x][y].content->has_work &&
         l->tiles[x][y].content->has_work(l->tiles[x][y].content->context);
}

TileContent *l_content(const Level *l, int x, int y) { return l_valid(l, x, y) ? l->tiles[x][y].content : NULL; }

static const float F = 16.0f;
float l_to_x(int i) { return i * F; }
float l_to_y(int j) { return j * F; }
Vec2 l_to_vec(int i, int j) { return (Vec2){i * F, j * F}; }
Vec2 l_to_vecP(Point p) { return l_to_vec(p.x, p.y); }
Rect l_to_vecR(Recti r) { return (Rect){l_to_vec(r.x, r.y), l_to_vec(r.w, r.h)}; }

Point l_to_point(Vec2 v) { return (Point){v.x / F, v.y / F}; }

typedef struct Queue {
  Point points[LEVEL_WIDTH * LEVEL_HEIGHT];
  int front, rear;
} Queue;

void q_init_queue(Queue *q) { q->front = q->rear = 0; }
bool q_is_queue_empty(Queue *q) { return q->front == q->rear; }
void q_enqueue(Queue *q, Point p) { q->points[q->rear++] = p; }
Point q_dequeue(Queue *q) { return q->points[q->front++]; }

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

  int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

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
