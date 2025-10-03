#ifndef GAME
#define GAME

#include <stdbool.h>
#include <stddef.h>

#include "engine/DrawTransformation.h"
#include "engine/Scene.h"
#include "engine/TextDrawEntity.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "game/assets.h"

const char *str(const char *fmt, ...);

typedef struct Game Game;

int g_main(const char *name, Scene scene);

void g_set_scene(Game *g, Scene scene);
void g_set_background_color(Game *g, Color c);

float g_animation_delta(Game *g);
float g_time(Game *g);

Sizei g_viewport(Game *g);

Vec2 g_mouse_in_scene(Game *g);
Vec2 g_mouse_on_overlay(Game *g);

TextDrawEntity *g_text(Game *g, G_Font ff);
void g_draw_text(Game *g, const TextDrawEntity *tde, Vec2 pan);

void g_draw_rect(Game *g, const DrawTransformation dt);

void g_draw_icon(Game *g, Image tex, int frame, const DrawTransformation dt);

void g_color(Game *game, Color c);

void *g_malloc(size_t size);
void *g_realloc(void *ptr, size_t size);

void c_color(Game *g, Color c);
void c_printf(Game *g, const char *fmt, ...);

#endif