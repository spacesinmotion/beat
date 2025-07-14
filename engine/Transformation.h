#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include "math/Vec2.h"

typedef struct Transformation {
  Vec2 position;
  float rotation;
  Vec2 scale;
} Transformation;

static inline const Transformation *t_PRS(Vec2 pos, float rot, Vec2 scale) {
  static Transformation t = {.position = (Vec2){}, .rotation = 0.0f, .scale = (Vec2){1.0f, 1.0f}};
  t.position = pos;
  t.rotation = rot;
  t.scale = scale;
  return &t;
}
static inline const Transformation *t_PR(Vec2 pos, float rot) {
  static Transformation t = {.position = (Vec2){}, .rotation = 0.0f, .scale = (Vec2){1.0f, 1.0f}};
  t.position = pos;
  t.rotation = rot;
  return &t;
}
static inline const Transformation *t_PS(Vec2 pos, Vec2 scale) {
  static Transformation t = {.position = (Vec2){}, .rotation = 0.0f, .scale = (Vec2){1.0f, 1.0f}};
  t.position = pos;
  t.scale = scale;
  return &t;
}
static inline const Transformation *t_P(Vec2 pos) {
  static Transformation t = {.position = (Vec2){}, .rotation = 0.0f, .scale = (Vec2){1.0f, 1.0f}};
  t.position = pos;
  return &t;
}

#endif
