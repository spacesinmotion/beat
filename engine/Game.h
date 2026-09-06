#ifndef GAME
#define GAME

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "game/assets.h"

#include "Scene.h"

typedef struct Game Game;

const char *str(const char *fmt, ...);

typedef struct G_Object {
  uint32_t vertices, indices, num_elements;
} G_Object;

typedef bool (*IsSetCB)(void *data, int i, int j);
G_Object create_tile_rect_buffer(int ni, int nj, IsSetCB is_set, void *data);
bool G_Object_valid(const G_Object *);
void G_Object_free(G_Object *);

// Text rendering
typedef struct FontDesc {
  const char *file;
  int size;
} FontDesc;
void g_create_font_list(Game *g, const FontDesc *fonts, int n);

typedef struct G_Text {
  G_Object buffer;
  int font_index;
} G_Text;
void g_create_text(Game *g, G_Text *t, int font_index, const char *text);
void g_text(Game *g, const G_Text *t, Vec2 pan);

void g_set_scene(Game *g, Scene scene);
void g_set_background_color(Game *g, Color c);

float g_animation_delta(Game *g);
float g_time(Game *g);
int g_frame(Game *g);

Sizei g_viewport(Game *g);

G_Object g_tilerect_buffer(Game *g, int w, int h);
G_Object g_animation_buffer(Game *g);

void g_color(Game *game, Color c);

void g_buffer(Game *g, G_Object buffer, Image tex, Vec2 pan);

void g_objectRS(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan, float rot, float scale);
void g_objectR(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan, float rot);
void g_objectS(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan, float scale);
void g_object(Game *g, G_Object buffer, Image tex, int frame, Vec2 pan);

void *g_malloc(size_t size);
void *g_realloc(void *ptr, size_t size);

void c_color(Game *g, Color c);
void c_printf(Game *g, const char *fmt, ...);

#endif