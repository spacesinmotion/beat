#ifndef BUILDINGDISPLAY_H
#define BUILDINGDISPLAY_H

#include "extern/cjsonh/cjsonh.h"
#include "game/Game.h"
#include "game/Level.h"
#include "math/Rect.h"
#include <math.h>

typedef struct BuildingDisplay {
  G_Object buffer;
  Recti location;

  float flash;

} BuildingDisplay;

BuildingDisplay bd_create(Game *g, Recti l) {
  return (BuildingDisplay){
      .buffer = g_tilerect_buffer(g, l.w, l.h),
      .location = l,
      .flash = 1.0f,
  };
}

void bd_update(BuildingDisplay *bd, Game *g) { bd->flash = f_max(0.0f, bd->flash - g_animation_delta(g)); }

void bd_draw(BuildingDisplay *bd, Game *g, Color c, MenuIcon icon) {
  Vec2 p = l_to_vecP(ri_bottom_right(bd->location));
  g_color(g, lighter(c, bd->flash * bd->flash));
  g_buffer(g, bd->buffer, Img_house_map, p);
  g_color(g, white());
  g_objectRS(g, g_animation_buffer(g), Img_menubar, icon, p, bd->flash * 0.2f * sin(17.0f * g_time(g)),
             1.0f + bd->flash * 0.2f * sin(26.0f * g_time(g)));
}

void bd_flash(BuildingDisplay *bd) { bd->flash = 1.0f; }

static inline void bd_to_json(CJHObject *o, BuildingDisplay *bd) {
  cjh_o_add_array(o, "location", (CJHWriteArrayCB)ri_to_json, &bd->location);
}
#endif