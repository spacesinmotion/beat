#ifndef GAME
#define GAME

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "game/assets.h"
#include "math/Color.h"
#include "math/Rect.h"
#include "math/Vec2.h"

#include "Scene.h"

const char *str(const char *fmt, ...);

typedef struct Transformation Transformation;
typedef struct sg_image sg_image;

typedef struct G_Object {
  uint32_t pipeline, vertices, indices, off_elements, num_elements;
} G_Object;

typedef bool (*IsSetCB)(void *data, int i, int j);
Vec2 g_create_text(Game *g, G_Object *o, G_Font ff, const char *text);
bool G_Object_valid(const G_Object *);
void G_Object_free(G_Object *);

typedef struct Game Game;

void g_set_scene(Game *g, Scene scene);
void g_set_background_color(Game *g, Color c);

float g_animation_delta(Game *g);
float g_time(Game *g);
int g_frame(Game *g);

Sizei g_viewport(Game *g);

G_Object g_animation_buffer(Game *g, int frame);
G_Object g_rect_buffer(Game *g);

void d_color(Game *game, Color c);

void d_object(Game *g, G_Object buffer, const sg_image texture, const Transformation *t);
void d_text(Game *g, G_Object buffer, G_Font f, Vec2 pan);
void d_rect(Game *g, Color c, const Transformation *t);
void d_image(Game *g, Image tex, const Transformation *t);
void d_animation(Game *g, Image tex, int frame, const Transformation *t);

void *g_malloc(size_t size);
void *g_realloc(void *ptr, size_t size);

void c_color(Game *g, Color c);
void c_printf(Game *g, const char *fmt, ...);

#endif