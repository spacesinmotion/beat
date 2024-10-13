

typedef struct Color {
  float r, g, b, a;
} Color;

static inline Color rgb(int r, int g, int b) { return (Color){r / 255.0f, g / 255.0f, b / 255.0f, 1.0f}; }
static inline Color rgba(int r, int g, int b, int a) { return (Color){r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f}; }
static inline Color gray(int g) { return rgb(g, g, g); }

static inline Color white() { return rgb(255, 255, 255); }
static inline Color red() { return rgb(255, 0, 0); }
static inline Color green() { return rgb(0, 255, 0); }
static inline Color blue() { return rgb(0, 0, 255); }

// fn lighter(c Color, t f32) Color {
//   return Color {
//     (c.r as f32 * (1.0f - t) + 255.0f * t) as i32, (c.g as f32 * (1.0f - t) + 255.0f * t) as i32,
//         (c.b as f32 * (1.0f - t) + 255.0f * t) as i32, c.a
//   }
// }

// fn alpha(c Color, a i32) Color {
//   return Color { c.r, c.g, c.b, a }
// }

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