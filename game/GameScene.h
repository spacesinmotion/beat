#ifndef GAME_SCENE
#define GAME_SCENE

#include <math.h>

#include "Game.h"

typedef struct GameScene {
  int wearisome_frame;
} GameScene;

static void GameScene_draw(Game *g, GameScene *scene) {
  (void)scene;

  d_noise(g, 0.01f);
  d_buffer(g, d_tilemap_buffer(g), d_tilemap_image(g), (Vec2){-8, 8});

  float time = Game_time(g);
  const int frame = (int)(time * 4);
  if (frame % 32 > 29)
    scene->wearisome_frame = 1;
  else
    scene->wearisome_frame = 0;
  d_noise(g, 0.1f);
  d_color(g, 0.01f * sin(time * 20) + 0.9f, 1.0f, 1.0f, 1.0f);
  d_object(g, d_animation_buffer(g), d_animation_image(g), (Vec2){16 + (int)(16 * sin(time)), 0.0f},
           scene->wearisome_frame);
}

void GameScene_init(Game *g) { game_set_scene(g, (SceneDrawCB)GameScene_draw, gc_malloc(&gc, sizeof(GameScene))); }
#endif