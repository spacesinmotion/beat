#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define LEVEL_WIDTH 48
#define LEVEL_HEIGHT 32

typedef struct Level {
  uint8_t tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} Level;

void Level_init(Level *level) { memset(level->tiles, 0, sizeof(level->tiles)); }

uint8_t Level_tile(Level *level, int x, int y) {
  if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT)
    return 0;
  return level->tiles[x][y];
}
void Level_set_tile(Level *level, int x, int y, uint8_t tile) {
  if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT)
    return;
  level->tiles[x][y] = tile;
}

void Level_clear_paths(Level *level) {
  for (int i = 0; i < LEVEL_WIDTH; ++i) {
    for (int j = 0; j < LEVEL_HEIGHT; ++j) {
      if (level->tiles[i][j] > 0)
        level->tiles[i][j] = 2;
    }
  }
}

typedef struct {
  int x, y;
} Point;

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
