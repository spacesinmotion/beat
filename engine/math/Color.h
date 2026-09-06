#ifndef COLOR
#define COLOR

#include "engine/extern/cjsonh/cjsonh.h"
#include <math.h>
typedef struct Color {
  float r, g, b, a;
} Color;

static inline Color rgb(int r, int g, int b) { return (Color){r / 255.0f, g / 255.0f, b / 255.0f, 1.0f}; }
static inline Color rgba(int r, int g, int b, int a) { return (Color){r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f}; }
static inline Color gray(int g) { return rgb(g, g, g); }

static inline Color white() { return rgb(255, 255, 255); }
static inline Color red() { return rgb(255, 0, 0); }
static inline Color yellow() { return rgb(255, 255, 0); }
static inline Color green() { return rgb(0, 255, 0); }
static inline Color blue() { return rgb(0, 0, 255); }

static inline Color warn_color() { return rgb(148, 116, 29); }
static inline Color critical_color() { return rgb(177, 53, 30); }

static inline Color warn(float t) {
  if (t > 0.5)
    return rgb(77, 213, 30);
  if (t > 0.25)
    return warn_color();
  return critical_color();
}

static inline Color lighter(Color c, float t) {
  return (Color){
      (c.r * (1.0f - t) + t),
      (c.g * (1.0f - t) + t),
      (c.b * (1.0f - t) + t),
      c.a,
  };
}

static inline Color c_mix(Color c1, Color c2, float t) {
  return (Color){
      (c1.r + t * (c2.r - c1.r)),
      (c1.g + t * (c2.g - c1.g)),
      (c1.b + t * (c2.b - c1.b)),
      (c1.a + t * (c2.a - c1.a)),
  };
}

static inline Color alphaf(Color c, float a) { return (Color){c.r, c.g, c.b, a}; }

static inline void c_to_json(CJHArray *a, const Color *c) {
  cjh_a_add_number(a, roundf(c->r * 255.0f));
  cjh_a_add_number(a, roundf(c->g * 255.0f));
  cjh_a_add_number(a, roundf(c->b * 255.0f));
  cjh_a_add_number(a, roundf(c->a * 255.0f));
}

static inline void c_from_json(CJHArrayR *a, int index, Color *c) {
  if (index == 0)
    // ri->x = cjh_a_read_number(a);
    printf("%.*sr: %g\n", indent, space, cjh_a_read_number(a) / 255.0);
  else if (index == 1)
    // ri->y = cjh_a_read_number(a);
    printf("%.*sg: %g\n", indent, space, cjh_a_read_number(a) / 255.0);
  else if (index == 2)
    // ri->w = cjh_a_read_number(a);
    printf("%.*sb: %g\n", indent, space, cjh_a_read_number(a) / 255.0);
  else if (index == 3)
    // ri->w = cjh_a_read_number(a);
    printf("%.*sa: %g\n", indent, space, cjh_a_read_number(a) / 255.0);
  else
    assert(false);
}

#endif