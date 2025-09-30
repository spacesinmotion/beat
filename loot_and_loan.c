

#include "engine/Game.impl.h"

#include "LootAndLoan/DungeonScene.h"

int main(int argc, char *argv[]) {
  gc_start(&gc, &argc);

  int result = g_main(DungeonScene_create());

  gc_stop(&gc);

  return result;
}