#ifndef CONSTRUCTIONMATERIALFACTORY_H
#define CONSTRUCTIONMATERIALFACTORY_H

#include "game/BuildingDisplay.h"
#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include <assert.h>

typedef struct ConstructionMaterialFactory {
  WorkProvider work_provider;

  int last_day_delivered;

  BuildingDisplay display;
} ConstructionMaterialFactory;

Color cmf_color() { return rgb(85, 84, 80); }

bool cmf_dead(ConstructionMaterialFactory *cmf) {
  (void)cmf;
  return false;
}

float cmf_render_order(ConstructionMaterialFactory *cmf) { return l_to_y(cmf->display.location.y); }

void cmf_update(ConstructionMaterialFactory *cmf, GameScene *gs, Game *g, float dt) {
  bd_update(&cmf->display, g);

  if (gs->day > cmf->last_day_delivered) {
    cmf->last_day_delivered = gs->day;
    while (cmf->work_provider.clicks_done > 0 &&
           gs->resource_pool.construction_material + 1 <= gs->resource_pool_max.construction_material) {
      gs->resource_pool.construction_material++;
      cmf->work_provider.clicks--;
      cmf->work_provider.clicks_claimed--;
      cmf->work_provider.clicks_work--;
      cmf->work_provider.clicks_done--;
    }
  }
}

void cmf_draw(ConstructionMaterialFactory *cmf, GameScene *gs, Game *g) {
  if (ri_contains(cmf->display.location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionMaterialFactory (%d,%d,%d,%d)\n", cmf->display.location.x, cmf->display.location.y, 4,
             3);
    c_printf(g, "----------------------\n");
  }

  bd_draw(&cmf->display, g, cmf_color(), MI_ConstructionMaterial);
  Vec2 p = l_to_vecP(ri_bottom_right(cmf->display.location));
  // if (cmf->temporary_deliver_timer > 0.0f)
  //   g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

  wp_draw_click_fields(&cmf->work_provider, g, v_add(p, l_to_vec(1, 1)), false);
}

static SceneObjectTable ConstructionMaterialFactory_table = {
    .dead = (SceneObjectDeadCB)cmf_dead,
    .render_order = (SceneObjectRenderOrderCB)cmf_render_order,
    .update = (SceneObjectUpdateCB)cmf_update,
    .draw = (SceneObjectDrawCB)cmf_draw,
};

ConstructionMaterialFactory *ConstructionMaterialFactory_init(Game *g, GameScene *gs, Point p) {
  ConstructionMaterialFactory *cmf = g_malloc(g, sizeof(ConstructionMaterialFactory));
  *cmf = (ConstructionMaterialFactory){
      .display = bd_create(g, (Recti){p.x, p.y, 3, 2}),
      .last_day_delivered = gs->day,
  };
  assert((void *)cmf == (void *)&cmf->work_provider);
  wp_init(&cmf->work_provider, 2, 1, 10.0f);

  l_set_tileR(gs->level, cmf->display.location, T_ConstructionMaterialFactory);
  l_set_tile_contentR(gs->level, cmf->display.location, to_TileContent(cmf, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = cmf, &ConstructionMaterialFactory_table});
  return cmf;
}

#endif