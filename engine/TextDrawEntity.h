#ifndef TEXTDRAWENTITY_H
#define TEXTDRAWENTITY_H

#include <stdbool.h>

typedef struct Game Game;
typedef struct TextDrawEntity TextDrawEntity;

bool tde_valid(const TextDrawEntity *tde);
void tde_free(Game *g, TextDrawEntity *tde);

void tde_set_text(TextDrawEntity *o, const char *text);

#ifdef GAME_ENGINE_IMPL

#include "engine/DrawEntity.h"

typedef struct FontImage FontImage;
typedef struct TextDrawEntity {
  DrawEntity draw_entity;
  const FontImage *font;
} TextDrawEntity;

bool tde_valid(const TextDrawEntity *tde) { return de_valid(&tde->draw_entity) && tde->font != NULL; }
void tde_free(Game *g, TextDrawEntity *tde) {
  (void)g;
  de_free(&tde->draw_entity);
  free(tde);
}

#endif

#endif