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
typedef void (*SceneMouseMoveCB)(void *, Game *, Vec2, Vec2);
typedef void (*SceneMouseCB)(void *, Game *, Vec2, Vec2, int);
typedef struct Scene {
  void *context;
  SceneUpdateCB update;
  SceneDrawCB draw;
  SceneDrawCB draw_overlay;
  SceneMouseMoveCB mouse_move;
  SceneMouseCB mouse_down;
  SceneMouseCB mouse_up;
} Scene;
void g_set_scene(Game *g, Scene scene);

float g_time(Game *g);
int g_frame(Game *g);

const Buffer *g_tilemap_buffer(Game *g);
const Buffer *g_animation_buffer(Game *g);

const sg_image *g_image(Game *g, Image i);

void g_color(Game *game, Color c);
void g_noise(Game *game, float n);

void g_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan);
void g_object(Game *g, const Buffer *buffer, const sg_image *tex, Vec2 pan, float rot, int frame);

int map_key(int i, int j);
void set_map_key(int i, int j, int k);

bool map_is_set(int i, int j);

#endif