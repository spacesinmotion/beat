#ifndef POINTOVERVIEW_H
#define POINTOVERVIEW_H

#include "Game.h"
#include "game/GameTypes.h"

typedef struct GroupCounter {
  G_Object text_3to5, text_6, text_g1, text_g2, text_points;
  int g1, g1_cache, g2, g2_cache, points, points_cache;
} GroupCounter;

typedef struct PointOverview {
  GroupCounter house;
  GroupCounter trees;
  GroupCounter animals;
  GroupCounter flowers;

  G_Object text_water_points;
  G_Object text_water_count;
  int water_count, water_count_cache;

  G_Object text_points;
  int points, points_cache;
} PointOverview;

void po_init_group_counter(GroupCounter *gc, Game *g) {
  g_create_text(g, &gc->text_3to5, Oswald_Regular_8, "3-5");
  g_create_text(g, &gc->text_6, Oswald_Regular_8, "6+");
  gc->g1_cache = gc->g2_cache = gc->points_cache = -1;
}

void po_update_group_counter(GroupCounter *gc, Game *g) {
  if (gc->g1 != gc->g1_cache) {
    g_create_text(g, &gc->text_g1, Oswald_Regular_8, str("10x%d", gc->g1));
    gc->g1_cache = gc->g1;
  }
  if (gc->g2 != gc->g2_cache) {
    g_create_text(g, &gc->text_g2, Oswald_Regular_8, str("25x%d", gc->g2));
    gc->g2_cache = gc->g2;
  }
  if (gc->points != gc->points_cache) {
    g_create_text(g, &gc->text_points, Oswald_Regular_12, str("%d", gc->points));
    gc->points_cache = gc->points;
  }
}

void po_draw_group_counter(GroupCounter *gc, Game *g, Vec2 p, ObjectType icon) {
  g_color(g, gray(170));
  g_objectRS(g, g_animation_buffer(g), Img_menubar, icon, p, 0.0, 0.7f);
  g_text(g, gc->text_3to5, Oswald_Regular_8, v_add(p, (Vec2){5, -3}));
  g_objectRS(g, g_animation_buffer(g), Img_menubar, icon, v_add(p, (Vec2){20, 0}), 0.0, 0.7f);
  g_text(g, gc->text_6, Oswald_Regular_8, v_add(p, (Vec2){25, -3}));

  g_text(g, gc->text_g1, Oswald_Regular_8, v_add(p, (Vec2){-4, -10}));
  g_text(g, gc->text_g2, Oswald_Regular_8, v_add(p, (Vec2){16, -10}));

  g_color(g, gray(220));
  g_text(g, gc->text_points, Oswald_Regular_12, v_add(p, (Vec2){35, -6}));
}

void po_update(PointOverview *po, Game *g) {
  po_update_group_counter(&po->house, g);
  po_update_group_counter(&po->trees, g);
  po_update_group_counter(&po->animals, g);
  po_update_group_counter(&po->flowers, g);

  if (po->water_count != po->water_count_cache) {
    g_create_text(g, &po->text_water_count, Oswald_Regular_8, str("15x%d", po->water_count));
    g_create_text(g, &po->text_water_points, Oswald_Regular_12, str("%d", 15 * po->water_count));
    po->water_count_cache = po->water_count;
  }
  if (po->points != po->points_cache) {
    g_create_text(g, &po->text_points, Oswald_Regular_12, str("%d", po->points));
    po->points_cache = po->points;
  }
}

void po_draw(PointOverview *po, Game *g) {
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
  g_objectRS(g, g_animation_buffer(g), Img_menubar, So_Water, v_add(p, (Vec2){0, 0}), 0.0, 0.7f);
  g_objectRS(g, g_animation_buffer(g), Img_menubar, So_None, v_add(p, (Vec2){10, 0}), 0.0, 0.7f);
  g_text(g, po->text_water_count, Oswald_Regular_8, v_add(p, (Vec2){-4, -10}));
  g_color(g, gray(220));
  g_text(g, po->text_water_points, Oswald_Regular_12, v_add(p, (Vec2){35, -6}));

  g_color(g, white());
  g_text(g, po->text_points, Oswald_Regular_12, (Vec2){l + 35, t - 10 - (o * row++)});
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

PointOverview *PointOverview_init(Game *g) {
  PointOverview *po = g_malloc(sizeof(PointOverview));
  *po = (PointOverview){0};

  po_init_group_counter(&po->house, g);
  po_init_group_counter(&po->trees, g);
  po_init_group_counter(&po->animals, g);
  po_init_group_counter(&po->flowers, g);

  // Initialize cache values to -1 to trigger initial text creation
  po->water_count_cache = po->points_cache = -1;

  return po;
}

#endif