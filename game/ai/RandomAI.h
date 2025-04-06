#ifndef RANDOMAI_H
#define RANDOMAI_H

#include "game/ConstructionSite.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/buildings/Altar.h"
#include "game/buildings/Archers.h"
#include "game/buildings/Castle.h"
#include "game/buildings/Farm.h"
#include "game/buildings/Forge.h"
#include "game/buildings/MenAtArms.h"
#include "game/buildings/Mine.h"
#include "game/buildings/Tower.h"
#include "game/buildings/Woodcutter.h"

typedef struct BuildingOption {
  MenuIcon key;
  Color color;
  Sizei size;
} BuildingOption;

// static inline bool ai_placeable(Level *l, int i, int j) {
//   const bool free = l_free(l, i, j);
//   const bool nearby = !l_free(l, i - 1, j) || !l_free(l, i + 1, j) || !l_free(l, i, j - 1) || !l_free(l, i, j + 1);
//   return free && nearby && j >= LEVEL_HEIGHT / 2;
// }
// static inline bool ai_contains_placeable(Level *l, Recti r) {
//   for (int i = r.x; i < r.x + r.w; ++i)
//     for (int j = r.y; j < r.y + r.h; ++j)
//       if (ai_placeable(l, i, j))
//         return true;
//   return false;
// }
// bool ai_construction_available(GameScene *gs, Recti r) {
//   if (!gs->game_paused || !l_freeR(gs->level, r))
//     return false;
//   return gs->enemy.resources.money > 0 && ai_contains_placeable(gs->level, r) && l_on_enemy_field(gs->level, r);
// }

bool gs_construction_available(GameScene *gs, Player *player, Recti r);

void ai_turn(GameScene *gs, Game *g) {
  (void)g;

  BuildingOption building_option[9] = {
      {MI_Castle, cs_color(), cs_size()},      {MI_Farm, fa_color(), fa_size()},
      {MI_WoodCutter, wc_color(), wc_size()},  {MI_Mine, mi_color(), mi_size()},
      {MI_MenAtArms, maa_color(), maa_size()}, {MI_Archers, ar_color(), ar_size()},
      {MI_Tower, to_color(), to_size()},       {MI_Altar, al_color(), al_size()},
      {MI_Forge, fo_color(), fo_size()},
  };

  for (int try = 0; try < 50; ++try) {
    const int b = rand() % 9;

    BuildingOption *bo = &building_option[b];
    Recti r = {rand() % LEVEL_WIDTH, rand() % LEVEL_HEIGHT, bo->size.w, bo->size.h};

    if (!gs_construction_available(gs, &gs->enemy, r))
      continue;

    ConstructionSite_init(gs, r, bo->key);
    if (rand() % 5 < 3)
      break;
  }
}

#endif