#ifndef MARKETPLACE_H
#define MARKETPLACE_H

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/SceneObject.h"
#include "game/TileContent.h"
#include "game/assets.h"
#include "math/Rect.h"

typedef struct Marketplace {
  G_Object buffer;
  Recti location;
} Marketplace;

Color mp_color() { return rgb(196, 113, 65); }

bool mp_dead(Marketplace *mp) {
  (void)mp;
  return false;
}

float mp_render_order(Marketplace *mp) { return l_to_y(mp->location.y); }

void mp_draw(Marketplace *mp, GameScene *gs, Game *g) {
  if (ri_contains(mp->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  Marketplace (%d,%d,%d,%d)\n", mp->location.x, mp->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(mp->location));
  g_color(g, mp_color());

  g_buffer(g, mp->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, MI_Marketplace, p);
}

static SceneObjectTable Marketplace_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)mp_dead,
    .render_order = (SceneObjectRenderOrderCB)mp_render_order,
    .draw = (SceneObjectDrawCB)mp_draw,
};

bool mp_provides(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;

  if (r == R_Water)
    return gs->resource_pool.water - gs->resource_pool_claimed.water > 0;
  else if (r == R_Food)
    return gs->resource_pool.food - gs->resource_pool_claimed.food > 0;
  return false;
}

void mp_claim(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;
  if (r == R_Water)
    gs->resource_pool_claimed.water++;
  else if (r == R_Food)
    gs->resource_pool_claimed.food++;
}

float mp_start(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;
  (void)gs;
  (void)r;
  return 0.1f;
}

void mp_done(Marketplace *mp, GameScene *gs, Resource r) {
  (void)mp;
  if (r == R_Water) {
    gs->resource_pool_claimed.water--;
    gs->resource_pool.water--;
  } else if (r == R_Food) {
    gs->resource_pool_claimed.food--;
    gs->resource_pool.food--;
  }
}

static TileContentTable Marketplace_TileContent_Table = {
    .provides = (ProvidesCB)mp_provides,
    .claim = (ClaimCB)mp_claim,
    .start = (StartCB)mp_start,
    .done = (DoneCB)mp_done,
};

Marketplace *Marketplace_init(Game *g, GameScene *gs, Point p) {
  Marketplace *mp = g_malloc(g, sizeof(Marketplace));
  *mp = (Marketplace){
      .buffer = g_tilerect_buffer(g, 4, 3),
      .location = (Recti){p.x, p.y, 4, 3},
  };

  l_set_tileR(gs->level, mp->location, T_Marketplace);
  l_set_tile_contentR(gs->level, mp->location, to_TileContent(mp, &Marketplace_TileContent_Table));

  gs_add_object(gs, (SceneObject){.context = mp, &Marketplace_table});
  return mp;
}
#endif // MARKETPLACE_H