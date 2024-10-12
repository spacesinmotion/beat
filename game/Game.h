#ifndef GAME
#define GAME

#include "game/assets.h"
#include "math/Vec2.h"

typedef struct sg_image sg_image;
typedef struct Buffer Buffer;

typedef struct Game Game;

typedef void (*SceneUpdateCB)(void *, Game *, float);
typedef void (*SceneDrawCB)(void *, Game *);
typedef struct Scene {
  void *context;
  SceneUpdateCB update;
  SceneDrawCB draw;
} Scene;
void game_set_scene(Game *g, Scene scene);

float Game_time(Game *g);

const Buffer *d_tilemap_buffer(Game *g);
const Buffer *d_animation_buffer(Game *g);

const sg_image *g_image(Game *g, Image i);

void d_color(Game *game, float r, float g, float b, float a);
void d_noise(Game *game, float n);

void d_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan);
void d_object(Game *g, const Buffer *buffer, const sg_image *tex, Vec2 pan, int frame);

#endif