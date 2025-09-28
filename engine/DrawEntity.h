#ifndef DRAWENTITY_H
#define DRAWENTITY_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Game Game;

typedef struct DrawEntity {
  uint32_t color_mode, vertices, indices, num_elements;
} DrawEntity;

bool de_valid(const DrawEntity *);
void de_free(DrawEntity *);

#ifdef GAME_ENGINE_IMPL

static inline bool de_valid(const DrawEntity *b) { return b->vertices > 0 && b->indices > 0 && b->num_elements > 0; }

static inline void de_free(DrawEntity *b) {
  sg_destroy_buffer((sg_buffer){b->vertices});
  sg_destroy_buffer((sg_buffer){b->indices});
  b->vertices = b->indices = b->num_elements = 0;
}

#endif

#endif