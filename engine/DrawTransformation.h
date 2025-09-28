#ifndef DRAWTRANSFORMATION_H
#define DRAWTRANSFORMATION_H

#include "engine/math/Vec2.h"

typedef struct DrawTransformation {
  Vec2 pan, scale;
  float rot;
} DrawTransformation;

static inline DrawTransformation dt_identity() { return (DrawTransformation){{0, 0}, {1, 1}, 0}; }
static inline DrawTransformation dt_p(Vec2 p) { return (DrawTransformation){p, {1, 1}, 0}; }
static inline DrawTransformation dt_r(float r) { return (DrawTransformation){{0, 0}, {1, 1}, r}; }
static inline DrawTransformation dt_s(Vec2 s) { return (DrawTransformation){{0, 0}, s, 0}; }
static inline DrawTransformation dt_sf(float s) { return (DrawTransformation){{0, 0}, {s, s}, 0}; }

static inline DrawTransformation dt_pr(Vec2 p, float r) { return (DrawTransformation){p, {1, 1}, r}; }

static inline DrawTransformation dt_ps(Vec2 p, Vec2 s) { return (DrawTransformation){p, s, 0}; }
static inline DrawTransformation dt_psf(Vec2 p, float s) { return (DrawTransformation){p, {s, s}, 0}; }

static inline DrawTransformation dt_prs(Vec2 p, float r, Vec2 s) { return (DrawTransformation){p, s, r}; }

static inline DrawTransformation dt_prsf(Vec2 p, float r, float s) { return (DrawTransformation){p, {s, s}, r}; }

#endif