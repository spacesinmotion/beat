#ifndef MAT4
#define MAT4

typedef struct Mat4 {
  float m[4][4];
} Mat4;

static inline Mat4 Mat4_orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far) {
  Mat4 O = (Mat4){{
      {1.0f, 0.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 0.0f, 1.0f},
  }};

  O.m[0][0] = 2.0f / (Right - Left);
  O.m[1][1] = 2.0f / (Top - Bottom);
  O.m[2][2] = 2.0f / (Near - Far);

  O.m[3][0] = (Left + Right) / (Left - Right);
  O.m[3][1] = (Bottom + Top) / (Bottom - Top);
  O.m[3][2] = (Far + Near) / (Near - Far);
  return O;
}

static inline Mat4 Mat4_mul(const Mat4 Left, const Mat4 Right) {
  Mat4 Result = {.m = {{0.0f}}};
  for (int c = 0; c < 4; ++c) {
    for (int r = 0; r < 4; ++r) {
      float Sum = 0.0f;
      for (int cm = 0; cm < 4; ++cm)
        Sum += Left.m[cm][r] * Right.m[c][cm];
      Result.m[c][r] = Sum;
    }
  }
  return Result;
}

static inline Mat4 Mat4_translation3f(float x, float y, float z) {
  Mat4 tr = {};
  tr.m[0][0] = tr.m[1][1] = tr.m[2][2] = tr.m[3][3] = 1.0f;
  tr.m[3][0] = x;
  tr.m[3][1] = y;
  tr.m[3][2] = z;
  return tr;
}

#endif