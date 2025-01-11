#ifndef CONSTRUCTIONMATERIALFACTORY_H
#define CONSTRUCTIONMATERIALFACTORY_H

#include "game/GameScene.h"
#include "game/Level.h"
#include "game/TileContent.h"
#include "game/WorkProvider.h"
#include "game/assets.h"
#include <assert.h>

typedef struct ConstructionMaterialFactory {
  WorkProvider work_provider;

  float temporary_deliver_timer;

  G_Object buffer;
  Recti location;
} ConstructionMaterialFactory;

Color cmf_color() { return rgb(85, 84, 80); }

bool cmf_dead(ConstructionMaterialFactory *cmf) {
  (void)cmf;
  return false;
}

float cmf_render_order(ConstructionMaterialFactory *cmf) { return l_to_y(cmf->location.y); }

void cmf_update(ConstructionMaterialFactory *cmf, GameScene *gs, Game *g, float dt) {
  (void)g;

  if (cmf->temporary_deliver_timer > 0.0f) {
    cmf->temporary_deliver_timer -= dt;
    if (cmf->temporary_deliver_timer <= 0.0f) {
      if (gs->resource_pool.construction_material + 2 <= gs->resource_pool_max.construction_material) {
        gs->resource_pool.construction_material += 2;
        wp_reset(&cmf->work_provider);
      } else
        cmf->temporary_deliver_timer = 1.0f;
    }
  } else if (wp_is_done(&cmf->work_provider)) {
    cmf->temporary_deliver_timer = 5.0;
  }
}

void cmf_draw(ConstructionMaterialFactory *cmf, GameScene *gs, Game *g) {
  if (ri_contains(cmf->location, gs->r.x, gs->r.y)) {
    c_printf(g, "----------------------\n");
    c_printf(g, "  ConstructionMaterialFactory (%d,%d,%d,%d)\n", cmf->location.x, cmf->location.y, 4, 3);
    c_printf(g, "----------------------\n");
  }

  Vec2 p = l_to_vecP(ri_bottom_right(cmf->location));
  g_color(g, cmf_color());
  g_buffer(g, cmf->buffer, Img_house_map, p);
  g_color(g, white());
  g_object(g, g_animation_buffer(g), Img_menubar, MI_ConstructionMaterial, p);
  if (cmf->temporary_deliver_timer > 0.0f)
    g_object(g, g_animation_buffer(g), Img_menubar, MI_Logistics, v_add(p, l_to_vec(0, 1)));

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
      .buffer = g_tilerect_buffer(g, 3, 2),
      .location = (Recti){p.x, p.y, 3, 2},
  };
  assert((void *)cmf == (void *)&cmf->work_provider);
  wp_init(&cmf->work_provider, 2, 1, 14.0f);

  l_set_tileR(gs->level, cmf->location, T_ConstructionMaterialFactory);
  l_set_tile_contentR(gs->level, cmf->location, to_TileContent(cmf, &WorkProvider_TileContent_Default_Table));

  gs_add_object(gs, (SceneObject){.context = cmf, &ConstructionMaterialFactory_table});
  return cmf;
}

#endif