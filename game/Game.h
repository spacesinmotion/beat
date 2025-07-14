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

typedef struct Game Game;

typedef struct RenderObject {
  uint32_t pipeline, vertices, indices, off_elements, num_elements;
} RenderObject;

bool RenderObject_valid(const RenderObject *);
void RenderObject_free(RenderObject *);

typedef struct TextObject {
  RenderObject render_object;
  G_Font font;
  Vec2 size;
} TextObject;
void g_create_text(Game *g, TextObject *o, G_Font ff, const char *text);
bool TextObject_valid(const TextObject *to);
void TextObject_free(TextObject *);

void g_set_scene(Game *g, Scene scene);
void g_set_background_color(Game *g, Color c);

float g_animation_delta(Game *g);
float g_time(Game *g);
static inline int g_frame(Game *g) { return (int)(g_time(g) * 16.0f); }

Sizei g_viewport(Game *g);

void d_color(Game *game, Color c);

void d_object(Game *g, RenderObject buffer, const sg_image texture, const Transformation *t);
void d_text(Game *g, const TextObject *to, Vec2 pan);
void d_rect(Game *g, Color c, const Transformation *t);
void d_image(Game *g, Image tex, const Transformation *t);
void d_animation(Game *g, Image tex, int frame, const Transformation *t);

void *g_malloc(size_t size);
void *g_realloc(void *ptr, size_t size);

void c_color(Game *g, Color c);
void c_printf(Game *g, const char *fmt, ...);

#endif