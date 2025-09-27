#ifndef GAMECOLORS_H
#define GAMECOLORS_H

#include "engine/Game.h"

static inline void clicked_color(Game *g) { g_color(g, rgb(143, 143, 143)); }
static inline void work_claimed_color(Game *g) { g_color(g, rgb(56, 85, 92)); }
static inline void working_color(Game *g) { g_color(g, rgb(89, 135, 146)); }
static inline void done_color(Game *g) { g_color(g, rgb(101, 168, 110)); }

#endif