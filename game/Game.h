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

typedef struct G_Object {
  uint32_t vertices, indices, num_elements;
} G_Object;

typedef bool (*IsSetCB)(void *data, int i, int j);
G_Object create_tile_rect_buffer(int ni, int nj, IsSetCB is_set, void *data);
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

G_Object g_animation_buffer(Game *g);
G_Object g_rect_buffer(Game *g);

void g_color(Game *game, Color c);

void g_buffer(Game *g, G_Object buffer, Image tex, Vec2 pan);

void g_objectRS(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan, float rot, Vec2 scale);
void g_objectR(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan, float rot);
void g_objectS(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan, Vec2 scale);
void g_object(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan);

void g_text(Game *g, G_Object buffer, G_Font f, Vec2 pan);

void *g_malloc(size_t size);
void *g_realloc(void *ptr, size_t size);

void c_color(Game *g, Color c);
void c_printf(Game *g, const char *fmt, ...);

#endif