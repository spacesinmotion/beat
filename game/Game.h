#ifndef GAME
#define GAME

#include "game/assets.h"
#include "math/Color.h"
#include "math/Vec2.h"

#include <stdbool.h>

typedef struct sg_image sg_image;
typedef struct Buffer Buffer;

typedef struct Game Game;

typedef void (*SceneUpdateCB)(void *, Game *, float);
typedef void (*SceneDrawCB)(void *, Game *);
typedef void (*SceneMouseMoveCB)(void *, Game *, Vec2);
typedef void (*SceneClickCB)(void *, Game *, Vec2, int);
typedef struct Scene {
  void *context;
  SceneUpdateCB update;
  SceneDrawCB draw;
  SceneMouseMoveCB mouse_move;
  SceneClickCB mouse_click;
} Scene;
void game_set_scene(Game *g, Scene scene);

float Game_time(Game *g);

const Buffer *d_tilemap_buffer(Game *g);
const Buffer *d_animation_buffer(Game *g);

const sg_image *g_image(Game *g, Image i);

void d_color(Game *game, Color c);
void d_noise(Game *game, float n);

void d_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan);
void d_object(Game *g, const Buffer *buffer, const sg_image *tex, Vec2 pan, int frame);

int map_key(int i, int j);
void set_map_key(int i, int j, int k);

bool map_is_set(int i, int j);

#endif