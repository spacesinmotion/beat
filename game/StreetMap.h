#ifndef STREETMAP
#define STREETMAP

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/MoveMarker.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Rect.h"
#include "math/Vec2.h"

#include <math.h>
#include <stdlib.h>

#define nii 20
#define njj 16
typedef struct StreetMap {
  const sg_image *texture;
  int maze_dir[nii][njj];
  int oi, oj;
} StreetMap;

bool StreetMap_dead(StreetMap *sm) {
  (void)sm;
  return false;
}

int street_tex_for(int i, int j) {
  int k = 0;
  if (map_key(i + 1, j) == 2)
    k += 1;
  if (map_key(i, j + 1) == 2)
    k += 2;
  if (map_key(i - 1, j) == 2)
    k += 4;
  if (map_key(i, j - 1) == 2)
    k += 8;
  return k;
}

void StreetMap_draw(StreetMap *sm, Game *g) {
  g_noise(g, 0.0f);
  g_color(g, white());

  for (int i = 0; i < nii; ++i) {
    for (int j = 0; j < njj; ++j) {
      if (i > 0 && sm->maze_dir[i - 1][j] == 0)
        g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 2);
      if (i < nii - 1 && sm->maze_dir[i + 1][j] == 2)
        g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 0);
      if (j > 0 && sm->maze_dir[i][j - 1] == 1)
        g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 3);
      if (j < nii - 1 && sm->maze_dir[i][j + 1] == 3)
        g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 1);
      g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 4);
      g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), sm->maze_dir[i][j]);
      g_object(g, g_animation_buffer(g), sm->texture, v_mulf((Vec2){i * 16, j * 16}, 1.0f), 8 + sm->maze_dir[i][j]);
    }
  }
}

void StreetMap_maze_step(StreetMap *sm) {
  for (int i = 0; i < 100; ++i) {
    const int d = rand() % 4;
    int noi = sm->oi, noj = sm->oj;
    if (d == 0)
      noi++;
    else if (d == 1)
      noj++;
    else if (d == 2)
      noi--;
    else if (d == 3)
      noj--;

    if (noi < 0 || noj < 0 || noi >= nii || noj >= njj)
      continue;

    sm->maze_dir[sm->oi][sm->oj] = d;
    sm->oi = noi;
    sm->oj = noj;
    sm->maze_dir[sm->oi][sm->oj] = 4;
    return;
  }
}

SceneObjectTable StreetMap_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)StreetMap_dead,
    .draw = (SceneObjectDrawCB)StreetMap_draw,
};
StreetMap *StreetMap_init(Game *g, GameScene *gs) {
  StreetMap *sm = gc_malloc(&gc, sizeof(StreetMap));
  *sm = (StreetMap){
      .texture = g_image(g, Img_maze_pointer),
      .oi = nii - 1,
      .oj = njj - 1,
  };

  for (int i = 0; i < nii; ++i) {
    for (int j = 0; j < njj; ++j) {
      sm->maze_dir[i][j] = i + 1 == nii ? 1 : 0;
    }
  }
  sm->maze_dir[sm->oi][sm->oj] = 4;
  for (int i = 0; i < 5000; ++i)
    StreetMap_maze_step(sm);

  GameScene_add_object(gs, (SceneObject){.context = sm, &StreetMap_table});
  return sm;
}

#endif