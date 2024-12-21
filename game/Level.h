#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT 48

typedef struct Level {
  uint8_t tiles[LEVEL_WIDTH][LEVEL_HEIGHT];
} Level;

uint8_t level_tile(Level *level, int x, int y) {
  if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT)
    return 0;
  return level->tiles[x][y];
}
void level_set_tile(Level *level, int x, int y, uint8_t tile) {
  if (x < 0 || x >= LEVEL_WIDTH || y < 0 || y >= LEVEL_HEIGHT)
    return;
  level->tiles[x][y] = tile;
}

typedef bool (*CanMoveCallback)(uint8_t tile);
typedef bool (*GoalReachedCallback)(int x, int y);

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

bool bfs(Level *level, int start_x, int start_y, CanMoveCallback can_move, GoalReachedCallback goal_reached) {
  Point predecessor[LEVEL_WIDTH][LEVEL_HEIGHT];
  memset(predecessor, -1, sizeof(predecessor));

  Queue q;
  init_queue(&q);
  enqueue(&q, (Point){start_x, start_y});
  predecessor[start_x][start_y] = (Point){start_x, start_y}; // self

  int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

  while (!is_queue_empty(&q)) {
    Point p = dequeue(&q);

    if (goal_reached(p.x, p.y)) {
      return true;
    }

    for (int i = 0; i < 4; ++i) {
      int nx = p.x + directions[i][0];
      int ny = p.y + directions[i][1];

      if (nx < 0 || nx >= LEVEL_WIDTH || ny < 0 || ny >= LEVEL_HEIGHT)
        continue;

      if (predecessor[nx][ny].x < 0 && can_move(level->tiles[nx][ny])) {
        predecessor[nx][ny] = (Point){p.x, p.y};
        enqueue(&q, (Point){nx, ny});
      }
    }
  }

  return false;
}

#endif // LEVEL_H
