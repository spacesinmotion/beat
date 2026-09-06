#ifndef FONTS
#define FONTS

#ifdef ENGINE_IMPLEMENTATION
#include "engine/FontDesc.h"
#endif

typedef enum GFont {
  Assistant_Regular_8 = 0,
  Assistant_Regular_12,
  Oswald_Regular_8,
  Oswald_Regular_12,
  Nb_Font,
} GFont;

#ifdef ENGINE_IMPLEMENTATION
static FontDesc font_list[Nb_Font] = {
    {"assets/Assistant-Regular.ttf", 8},
    {"assets/Assistant-Regular.ttf", 12},
    {"assets/Oswald-Regular.ttf", 8},
    {"assets/Oswald-Regular.ttf", 12},
};
#define G_FONT GFont
#define G_FONT_COUNT Nb_Font
#define G_FONT_LIST font_list
#endif

#endif