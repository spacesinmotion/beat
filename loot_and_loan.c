

#include "engine/Game.impl.h"

#include "LootAndLoan/DungeonScene.h"

int main(int argc, char *argv[]) {
  (void)argc, (void)argv;

  return g_main("Loot & Loan", DungeonScene_create);
}