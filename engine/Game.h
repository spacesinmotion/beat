#ifndef GAME
#define GAME

#include <stdbool.h>
#include <stddef.h>

#include "engine/DrawEntity.h"
#include "engine/DrawTransformation.h"
#include "engine/TextDrawEntity.h"
#include "engine/math/Color.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "engine/scene/Scene.h"
#include "game/assets.h"

const char *str(const char *fmt, ...);

typedef struct Game Game;

typedef void (*GameInitCB)(Game *g);
int g_main(const char *name, GameInitCB init);

void g_set_scene(Game *g, Scene scene);
void g_set_background_color(Game *g, Color c);

float g_animation_delta(Game *g);
float g_time(Game *g);

Sizei g_viewport(Game *g);

Vec2 g_mouse_in_scene(Game *g);
Vec2 g_mouse_on_overlay(Game *g);

DrawEntity *g_sub_image(Game *g, Sizei s, Point center, Point sub[4]);

TextDrawEntity *g_text(Game *g, G_Font ff);
void g_draw_text(Game *g, const TextDrawEntity *tde, Vec2 pan);

void g_draw_entity(Game *g, const DrawEntity *tde, Image tex, const DrawTransformation dt);

void g_draw_rect(Game *g, const DrawTransformation dt);

void g_draw_icon(Game *g, Image tex, int frame, const DrawTransformation dt);

void g_color(Game *g, Color c);

void *g_malloc(Game *g, size_t size);
void *g_realloc(Game *g, void *ptr, size_t size);
void g_free(Game *g, void *ptr);

void c_color(Game *g, Color c);
void c_printf(Game *g, const char *fmt, ...);

#endif