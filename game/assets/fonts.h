#ifndef FONTS
#define FONTS

#include "engine/Game.h"

typedef enum Font {
  Assistant_Regular_8 = 0,
  Assistant_Regular_12,
  Oswald_Regular_8,
  Oswald_Regular_12,
  Nb_Font,
} Font;

static FontDesc fonts[Nb_Font] = {
    {"assets/Assistant-Regular.ttf", 8},
    {"assets/Assistant-Regular.ttf", 12},
    {"assets/Oswald-Regular.ttf", 8},
    {"assets/Oswald-Regular.ttf", 12},
};
#endif