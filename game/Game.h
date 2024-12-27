#ifndef GAME
#define GAME

#include "game/assets.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"

#include <stdbool.h>

typedef struct sg_image sg_image;
typedef struct Buffer Buffer;

typedef bool (*IsSetCB)(void *data, int i, int j);
Buffer *create_tile_rect_buffer(int ni, int nj, IsSetCB is_set, void *data);
void Buffer_free(Buffer *b);

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

Point g_viewport(Game *g);

const Buffer *g_tilerect_buffer(Game *g, int w, int h);
const Buffer *g_animation_buffer(Game *g);

const sg_image *g_image(Game *g, Image i);

void g_color(Game *game, Color c);
void g_noise(Game *game, float n);

void g_buffer(Game *g, const Buffer *buffer, const sg_image *img, Vec2 pan);

void g_objectRS(Game *g, const Buffer *buffer, const sg_image *tex, int frame, Vec2 pan, float rot, float scale);
void g_objectR(Game *g, const Buffer *buffer, const sg_image *tex, int frame, Vec2 pan, float rot);
void g_objectS(Game *g, const Buffer *buffer, const sg_image *tex, int frame, Vec2 pan, float scale);
void g_object(Game *g, const Buffer *buffer, const sg_image *tex, int frame, Vec2 pan);

int map_key(int i, int j);
void set_map_key(int i, int j, int k);

bool map_is_set(int i, int j);

#endif