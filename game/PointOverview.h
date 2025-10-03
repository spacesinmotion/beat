#ifndef POINTOVERVIEW_H
#define POINTOVERVIEW_H

#include "engine/DrawTransformation.h"
#include "engine/Game.h"
#include "game/ObjectType.h"

typedef struct GroupCounter {
  TextDrawEntity *text_3to5, *text_6, *text_g1, *text_g2, *text_points;
  int g1, g1_cache, g2, g2_cache, points, points_cache;
} GroupCounter;

void po_gc_free(GroupCounter *gc) {
  tde_free(gc->text_3to5);
  tde_free(gc->text_6);
  tde_free(gc->text_g1);
  tde_free(gc->text_g2);
  tde_free(gc->text_points);
}

typedef struct PointOverview {
  GroupCounter house;
  GroupCounter trees;
  GroupCounter animals;
  GroupCounter flowers;

  TextDrawEntity *text_water_points, *text_water_count;
  int water_count, water_count_cache;

  TextDrawEntity *text_points;
  int points, points_cache;
} PointOverview;

void po_init_group_counter(GroupCounter *gc, Game *g) {
  gc->text_3to5 = g_text(g, Oswald_Regular_8);
  tde_set_text(gc->text_3to5, "3-5");
  gc->text_6 = g_text(g, Oswald_Regular_8);
  tde_set_text(gc->text_6, "6+");
  gc->text_g1 = g_text(g, Oswald_Regular_8);
  gc->text_g2 = g_text(g, Oswald_Regular_8);
  gc->text_points = g_text(g, Oswald_Regular_12);
  gc->g1_cache = gc->g2_cache = gc->points_cache = -1;
}

void po_update_group_counter(GroupCounter *gc) {
  if (gc->g1 != gc->g1_cache) {
    tde_set_text(gc->text_g1, str("10x%d", gc->g1));
    gc->g1_cache = gc->g1;
  }
  if (gc->g2 != gc->g2_cache) {
    tde_set_text(gc->text_g2, str("25x%d", gc->g2));
    gc->g2_cache = gc->g2;
  }
  if (gc->points != gc->points_cache) {
    tde_set_text(gc->text_points, str("%d", gc->points));
    gc->points_cache = gc->points;
  }
}

void po_draw_group_counter(GroupCounter *gc, Game *g, Vec2 p, ObjectType icon) {
  g_color(g, gray(170));
  g_draw_icon(g, Img_menubar, icon, dt_psf(p, 0.7f));
  g_draw_text(g, gc->text_3to5, v_add(p, (Vec2){5, -3}));
  g_draw_icon(g, Img_menubar, icon, dt_psf(v_add(p, (Vec2){20, 0}), 0.7f));
  g_draw_text(g, gc->text_6, v_add(p, (Vec2){25, -3}));

  g_draw_text(g, gc->text_g1, v_add(p, (Vec2){-4, -10}));
  g_draw_text(g, gc->text_g2, v_add(p, (Vec2){16, -10}));

  g_color(g, gray(220));
  g_draw_text(g, gc->text_points, v_add(p, (Vec2){35, -6}));
}

void po_update(PointOverview *po, Game *g) {
  (void)g;

  po_update_group_counter(&po->house);
  po_update_group_counter(&po->trees);
  po_update_group_counter(&po->animals);
  po_update_group_counter(&po->flowers);

  if (po->water_count != po->water_count_cache) {
    tde_set_text(po->text_water_count, str("15x%d", po->water_count));
    tde_set_text(po->text_water_points, str("%d", 15 * po->water_count));
    po->water_count_cache = po->water_count;
  }
  if (po->points != po->points_cache) {
    tde_set_text(po->text_points, str("%d", po->points));
    po->points_cache = po->points;
  }
}

void po_draw(PointOverview *po, Game *g, bool no_move_left) {
  int row = 0;
  const int l = 90;
  const int t = 100;
  const int o = 18;

  po_draw_group_counter(&po->house, g, (Vec2){l, t - (o * row++)}, So_House);
  po_draw_group_counter(&po->trees, g, (Vec2){l, t - (o * row++)}, So_Trees);
  po_draw_group_counter(&po->animals, g, (Vec2){l, t - (o * row++)}, So_Animals);
  po_draw_group_counter(&po->flowers, g, (Vec2){l, t - (o * row++)}, So_Flowers);

  Vec2 p = {l, t - (o * row++)};
  g_color(g, gray(170));
  g_draw_icon(g, Img_menubar, So_Water, dt_psf(v_add(p, (Vec2){0, 0}), 0.7f));
  g_draw_icon(g, Img_menubar, So_None, dt_psf(v_add(p, (Vec2){10, 0}), 0.7f));
  g_draw_text(g, po->text_water_count, v_add(p, (Vec2){-4, -10}));
  g_color(g, gray(220));
  g_draw_text(g, po->text_water_points, v_add(p, (Vec2){35, -6}));

  g_color(g, no_move_left ? red() : white());
  g_draw_text(g, po->text_points, (Vec2){l + 35, t - 10 - (o * row++)});
}

void po_group_counter_add_group(GroupCounter *gc, int c) {
  if (c >= 6) {
    gc->g2 += 1;
    gc->points += 25;
  } else if (c >= 3) {
    gc->g1 += 1;
    gc->points += 10;
  }
}

void po_count_points(PointOverview *po) {
  po->house.g1 = po->house.g2 = po->house.points = 0;
  po->trees.g1 = po->trees.g2 = po->trees.points = 0;
  po->animals.g1 = po->animals.g2 = po->animals.points = 0;
  po->flowers.g1 = po->flowers.g2 = po->flowers.points = 0;

  // Points are calculated by GameScene and set into PointOverview
  po->points = po->house.points + po->trees.points + po->animals.points + po->flowers.points + po->water_count * 15;
}

void po_free(PointOverview *po) {
  po_gc_free(&po->house);
  po_gc_free(&po->trees);
  po_gc_free(&po->animals);
  po_gc_free(&po->flowers);

  tde_free(po->text_water_count);
  tde_free(po->text_water_points);
  tde_free(po->text_points);
}

PointOverview *PointOverview_init(Game *g) {
  PointOverview *po = g_malloc(sizeof(PointOverview));
  *po = (PointOverview){.text_water_count = g_text(g, Oswald_Regular_8),
                        .text_water_points = g_text(g, Oswald_Regular_12),
                        .text_points = g_text(g, Oswald_Regular_12)};

  po_init_group_counter(&po->house, g);
  po_init_group_counter(&po->trees, g);
  po_init_group_counter(&po->animals, g);
  po_init_group_counter(&po->flowers, g);

  // Initialize cache values to -1 to trigger initial text creation
  po->water_count_cache = po->points_cache = -1;

  return po;
}

#endif