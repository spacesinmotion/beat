#ifndef STREETMAP
#define STREETMAP

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Vec2.h"

#include <stdlib.h>

#define nii 20
#define njj 16
typedef struct StreetMap {
  const sg_image *texture;
  Level *level;
} StreetMap;

bool StreetMap_dead(StreetMap *sm) {
  (void)sm;
  return false;
}

int street_tex_for(StreetMap *sm, int i, int j) {
  int k = 0;
  if (Level_tile(sm->level, i + 1, j) != 0)
    k += 1;
  if (Level_tile(sm->level, i, j + 1) != 0)
    k += 2;
  if (Level_tile(sm->level, i - 1, j) != 0)
    k += 4;
  if (Level_tile(sm->level, i, j - 1) != 0)
    k += 8;
  return k;
}

void StreetMap_draw(StreetMap *sm, Game *g) {
  g_noise(g, 0.0f);

  for (int i = 0; i < LEVEL_WIDTH; ++i) {
    for (int j = 0; j < LEVEL_WIDTH; ++j) {
      uint8_t tc = Level_tile(sm->level, i, j);
      if (tc == 0)
        continue;
      if (tc == 2)
        g_color(g, white());
      else if (tc == 3)
        g_color(g, red());
      else if (tc == 4)
        g_color(g, rgb(194, 130, 130));

      g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f,
               street_tex_for(sm, i, j));

      // if (i > 0 && sm->maze_dir[i - 1][j] == 0)
      //   g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f, 2);
      // if (i < nii - 1 && sm->maze_dir[i + 1][j] == 2)
      //   g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f, 0);
      // if (j > 0 && sm->maze_dir[i][j - 1] == 1)
      //   g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f, 3);
      // if (j < nii - 1 && sm->maze_dir[i][j + 1] == 3)
      //   g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f, 1);
      // g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f, 4);
      // g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0.0f,
      // sm->maze_dir[i][j]); g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f),
      // 0.0f,
      //          8 + sm->maze_dir[i][j]);
    }
  }
}

// void StreetMap_maze_step(StreetMap *sm) {
//   for (int i = 0; i < 100; ++i) {
//     const int d = rand() % 4;
//     int noi = sm->oi, noj = sm->oj;
//     if (d == 0)
//       noi++;
//     else if (d == 1)
//       noj++;
//     else if (d == 2)
//       noi--;
//     else if (d == 3)
//       noj--;

//     if (noi < 0 || noj < 0 || noi >= nii || noj >= njj)
//       continue;

//     sm->maze_dir[sm->oi][sm->oj] = d;
//     sm->oi = noi;
//     sm->oj = noj;
//     sm->maze_dir[sm->oi][sm->oj] = 4;
//     return;
//   }
// }

// SceneObjectTable StreetMap_table = (SceneObjectTable){
//     .dead = (SceneObjectDeadCB)StreetMap_dead,
//     .draw = (SceneObjectDrawCB)StreetMap_draw,
// };
StreetMap *StreetMap_init(Game *g, GameScene *gs) {
  StreetMap *sm = gc_malloc(&gc, sizeof(StreetMap));
  *sm = (StreetMap){
      .texture = g_image(g, Img_street),
      .level = gs->level,
  };

  int last_x = 2, last_y = 2;
  for (int n = 0; n < 13; ++n) {
    int x = 2 + rand() % (LEVEL_WIDTH - 4);
    int y = 2 + rand() % (LEVEL_HEIGHT - 4);

    int width = 3 + rand() % 6;
    int height = 3 + rand() % 6;
    int next_x = x + width / 2;
    int next_y = y + height / 2;

    for (int i = x; i < x + width && i < LEVEL_WIDTH; ++i) {
      for (int j = y; j < y + height && j < LEVEL_HEIGHT; ++j) {
        Level_set_tile(sm->level, i, j, 2);
      }
    }

    if (n > 0) {
      if (rand() % 2 == 0) {
        for (int i = i_min(last_y, next_y); i < i_max(last_y, next_y); ++i)
          Level_set_tile(sm->level, i_min(last_x, next_x), i, 2);
        for (int i = i_min(last_x, next_x); i < i_max(last_x, next_x); ++i)
          Level_set_tile(sm->level, i, last_x < next_x ? next_y : last_y, 2);
      } else {
        for (int i = i_min(last_x, next_x); i < i_max(last_x, next_x); ++i)
          Level_set_tile(sm->level, i, i_min(last_y, next_y), 2);
        for (int i = i_min(last_y, next_y); i < i_max(last_y, next_y); ++i)
          Level_set_tile(sm->level, last_y < next_y ? next_x : last_x, i, 2);
      }
    }
    last_x = next_x;
    last_y = next_y;
  }
  // for (int i = 0; i < nii; ++i) {
  //   for (int j = 0; j < njj; ++j) {
  //     sm->maze_dir[i][j] = i + 1 == nii ? 1 : 0;
  //   }
  // }
  // sm->maze_dir[sm->oi][sm->oj] = 4;
  // for (int i = 0; i < 5000; ++i)
  //   StreetMap_maze_step(sm);

  // GameScene_add_object(gs, (SceneObject){.context = sm, &StreetMap_table});
  return sm;
}

#endif