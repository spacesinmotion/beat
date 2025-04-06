#ifndef LEVEL_H
#define LEVEL_H

#include "extern/cjsonh/Z85.h"
#include "extern/cjsonh/cjsonh.h"
#include "game/TileContent.h"
#include "math/Rect.h"
#include "math/Vec2.h"
#include <stdbool.h>
#include <string.h>

#define LEVEL_WIDTH 16
#define LEVEL_HEIGHT 16

typedef struct TileContent TileContent;
typedef struct Tile {
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

bool l_free(Level *level, int x, int y) { return l_valid(level, x, y) && level->tiles[x][y].content == NULL; }
bool l_freeP(Level *level, Point p) { return l_free(level, p.x, p.y); }
bool l_freeR(Level *level, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (!l_free(level, i, j))
        return false;
  return true;
}

static inline bool l_placeable(Level *l, int i, int j) {
  const bool free = l_free(l, i, j);
  const bool nearby = !l_free(l, i - 1, j) || !l_free(l, i + 1, j) || !l_free(l, i, j - 1) || !l_free(l, i, j + 1);
  return free && nearby;
}
static inline bool l_contains_placeable(Level *l, Recti r) {
  for (int i = r.x; i < r.x + r.w; ++i)
    for (int j = r.y; j < r.y + r.h; ++j)
      if (l_placeable(l, i, j))
        return true;
  return false;
}
// static inline bool l_on_your_field(Level *level, Recti r) {
//   (void)level;
//   return r.y + r.h <= LEVEL_HEIGHT / 2;
// }
// static inline bool l_on_enemy_field(Level *level, Recti r) {
//   (void)level;
//   return r.y > LEVEL_HEIGHT / 2;
// }

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

void l_movable_to_json(CJHObject *o, Level *l) {
  unsigned short points[2 * LEVEL_WIDTH * LEVEL_HEIGHT];
  unsigned nb_entries = 0;

  const unsigned short sep = LEVEL_WIDTH;
  points[nb_entries++] = sep;
  for (int i = 0; i < LEVEL_WIDTH; ++i) {
    bool need_line = true;
    for (int j = 0; j < LEVEL_HEIGHT; ++j) {
      if (l_free(l, i, j)) {
        if (need_line) {
          points[nb_entries++] = (unsigned short)i;
          need_line = false;
        }
        points[nb_entries++] = (unsigned short)j;
      }
    }
    if (!need_line)
      points[nb_entries++] = sep;
  }
  if ((nb_entries * sizeof(short)) % 4 != 0)
    points[nb_entries++] = sep;

  char *z85 = Z85_encode((unsigned char *)points, nb_entries * sizeof(short));
  cjh_o_add_string(o, "movable", z85);
  // printf("shorts: %u, data: %llu, z85: %llu\n", nb_entries, nb_entries * sizeof(short), strlen(z85));
  free(z85);
}

void l_to_json(CJHObject *o, Level *l) {
  cjh_o_add_number_if(o, "width", LEVEL_WIDTH, 0);
  cjh_o_add_number_if(o, "height", LEVEL_HEIGHT, 0);
  l_movable_to_json(o, l);
}

void l_movable_from_json(CJHObjectR *o, Level *l) {
  (void)l;
  StrView str = cjh_o_read_string(o);
  // printf("%.*smovable: %.*s\n", indent, space, str.len, str.s);

  char *tmp = (char *)str.s;
  tmp[str.len] = '\0';
  size_t nb_entries = 0;
  unsigned short *points = (unsigned short *)Z85_decode(tmp, &nb_entries);
  nb_entries /= sizeof(unsigned short);
  tmp[str.len] = '"';

  assert(points);
  const unsigned short sep = points[0];
  for (size_t i = 1; i < nb_entries; ++i) {
    if (points[i] == sep)
      printf("\n");
    else if (points[i - 1] == sep)
      printf("%.*sline %d: ", indent, space, points[i]);
    else
      printf(" %d", points[i]);
  }

  free(points);
}

void l_from_json(CJHObjectR *o, const char *key, Level *l) {
  if (streq(key, "width"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "height"))
    printf("%.*s%s: %g\n", indent, space, key, cjh_o_read_number(o));
  else if (streq(key, "movable")) {
    indent += 2;
    l_movable_from_json(o, l);
    indent -= 2;
  } else {
    assert(false);
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

#endif // LEVEL_H
