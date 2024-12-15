#ifndef G_RANDOM
#define G_RANDOM

#include <stdlib.h>

static inline float r_float() { return (float)rand() / (float)RAND_MAX; }
static inline float r_float_r(float a, float b) { return a + r_float() * (b - a); }

#endif