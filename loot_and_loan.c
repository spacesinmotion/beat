

#include "engine/Game.impl.h"

#include "LootAndLoan/DungeonScene.h"

void init_loot_and_loan(Game *g) { g_set_scene(g, DungeonScene_create(g)); }

int main(int argc, char *argv[]) {
  (void)argc, (void)argv;

  return g_main("Loot & Loan", init_loot_and_loan);
}