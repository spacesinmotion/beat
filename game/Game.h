#ifndef GAME
#define GAME

#include "math/Vec2.h"

typedef struct sg_image sg_image;
typedef struct Buffer Buffer;

typedef struct Game Game;
typedef void (*SceneDrawCB)(Game *, void *);

void game_set_scene(Game *g, SceneDrawCB, void *scene);

float Game_time(Game *g);

const Buffer *d_tilemap_buffer(Game *g);
const sg_image *d_tilemap_image(Game *g);

const Buffer *d_animation_buffer(Game *g);
const sg_image *d_animation_image(Game *g);

void d_color(Game *game, float r, float g, float b, float a);
void d_noise(Game *game, float n);

void d_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan);
void d_object(Game *g, const Buffer *buffer, const sg_image *tex, Vec2 pan, int frame);

#endif