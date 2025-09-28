#ifndef TEXTDRAWENTITY_H
#define TEXTDRAWENTITY_H

#include "engine/DrawEntity.h"

typedef struct TextDrawEntity {
  DrawEntity draw_entity;
  uint32_t texture_id;
} TextDrawEntity;

bool tde_valid(const TextDrawEntity *tde);
void tde_free(TextDrawEntity *tde);

#ifdef GAME_ENGINE_IMPL

static inline bool tde_valid(const TextDrawEntity *tde) { return de_valid(&tde->draw_entity) && tde->texture_id > 0; }
static inline void tde_free(TextDrawEntity *tde) { return de_free(&tde->draw_entity); }

#endif

#endif