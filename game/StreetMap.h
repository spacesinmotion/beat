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

typedef struct StreetMap {
  const sg_image *texture;
} StreetMap;

bool StreetMap_dead(StreetMap *sm) {
  (void)sm;
  return false;
}

void StreetMap_draw(StreetMap *sm, Game *g) {
  d_noise(g, 0.0f);
  d_color(g, white());

  // const int nii = 20;
  // const int njj = 16;
  // for (int i = -1; i < nii; ++i) {
  //   for (int j = -1; j < njj; ++j) {
  //     d_object(g, d_animation_buffer(g), sm->texture, (Vec2){i * 16, j * 16}, 0);
  //   }
  // }
}

SceneObjectTable StreetMap_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)StreetMap_dead,
    .draw = (SceneObjectDrawCB)StreetMap_draw,
};
StreetMap *StreetMap_init(Game *g, GameScene *gs) {
  StreetMap *sm = gc_malloc(&gc, sizeof(StreetMap));
  *sm = (StreetMap){
      .texture = g_image(g, Img_street),
  };
  GameScene_add_object(gs, (SceneObject){.context = sm, &StreetMap_table});
  return sm;
}

#endif