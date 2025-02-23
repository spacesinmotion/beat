#ifndef COLOR
#define COLOR

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

static inline Color warn(float t) {
  if (t > 0.5)
    return rgb(77, 213, 30);
  if (t > 0.25)
    return rgb(216, 170, 43);
  return rgb(177, 53, 30);
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

// fn vec4(c Color) Vec4 {
//   return Vec4 { c.r as f32 / 255.0f, c.g as f32 / 255.0f, c.b as f32 / 255.0f, c.a as f32 / 255.0f, }
// }

// fn bg_color() Color{return rgba(65, 70, 70, 200)}

// fn highlight() Color{return rgb(187, 134, 74)}

// fn error_color() Color{return rgb(240, 105, 87)}

// fn good_color() Color{return rgb(87, 240, 168)}

// fn disabled_color() Color {
//   return rgb(99, 99, 99)
// }

#endif