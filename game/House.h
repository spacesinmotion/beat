#ifndef HOUSE_H
#define HOUSE_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/Wearisome.h"
#include "game/assets.h"
#include "math/Rect.h"

typedef struct House {
  G_Object buffer;
  Point location;

  struct {
    float food, water;
  } resources;

  Wearisome *wearisome;
} House;

Color House_color() { return rgb(87, 163, 106); }

bool House_dead(House *h) {
  (void)h;
  return false;
}

Circle House_circle(House *h) {
  (void)h;
  return (Circle){0};
}

Point House_current_entry(House *h, Level *l) {
  int options[8][2] = {{-1, 0}, {-1, 1}, {2, 0}, {2, 1}, {0, -1}, {1, -1}, {0, 2}, {1, 2}};
  for (int i = 0; i < 8; ++i) {
    int ii = h->location.x + options[i][0];
    int jj = h->location.y + options[i][1];
    if (Level_movable(l, ii, jj))
      return (Point){ii, jj};
  }
  return (Point){h->location.x, h->location.y};
}

void House_update(House *h, GameScene *gs, float dt) {
  (void)dt;
  (void)gs;

  if (h->wearisome && w_dead(h->wearisome))
    h->wearisome = NULL;

  if (h->wearisome && w_is_home(h->wearisome)) {
    w_sleep(h->wearisome, 6.0f * gs->daytime_step);

    h->resources.water -= w_drink(h->wearisome, f_min(h->resources.water, 8.0 * gs->daytime_step));
    h->resources.food -= w_eat(h->wearisome, f_min(h->resources.food, 8.0 * gs->daytime_step));
    printf("House w:%f f:%f ", h->resources.water, h->resources.food);
    Wearisome *w = h->wearisome;
    printf("Wearisome: %f (w:%f f:%f s:%f)\n", w->health, w->needs.water, w->needs.food, w->needs.sleep);
  }
}

void House_draw(House *h, Game *g) {
  g_color(g, h->wearisome ? House_color() : rgb(0, 0, 0));
  g_buffer(g, h->buffer, Img_house_map, Level_to_vecP(h->location));
}

static SceneObjectTable House_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)House_dead,
    .circle = (SceneObjectCircle)House_circle,
    .update = (SceneObjectUpdateCB)House_update,
    .draw = (SceneObjectDrawCB)House_draw,
};
House *House_init(Game *g, GameScene *gs, Point p) {
  House *h = g_malloc(g, sizeof(House));
  *h = (House){
      .buffer = g_tilerect_buffer(g, 2, 2),
      .location = p,
      .resources =
          {
              .food = 1.0f,
              .water = 2.0f,
          },
      .wearisome = NULL,
  };

  Recti r = (Recti){p.x, p.y, 2, 2};
  Level_set_tileR(gs->level, r, T_House);
  GameScene_add_object(gs, (SceneObject){h, &House_table});

  h->wearisome = Wearisome_init(g, gs, r);

  return h;
}
#endif // HOUSE_H