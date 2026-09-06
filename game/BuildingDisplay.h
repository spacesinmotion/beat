#ifndef BUILDINGDISPLAY_H
#define BUILDINGDISPLAY_H

#include "engine/Game.h"
#include "engine/extern/cjsonh/cjsonh.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"
#include "engine/math/random.h"
#include "game/Level.h"
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

void bd_draw_icon(Game *g, Vec2 p, int icon, float flash) {
  g_objectRS(g, g_animation_buffer(g), Img_menubar, icon, p, flash * 0.2f * sin(17.0f * g_time(g)),
             1.0f + flash * 0.2f * sin(26.0f * g_time(g)));
}
void bd_draw(BuildingDisplay *bd, Game *g, Color c, MenuIcon icon) {
  Vec2 p = l_to_vecP(ri_bottom_right(bd->location));
  g_color(g, lighter(c, bd->flash * bd->flash));
  g_buffer(g, bd->buffer, Img_house_map, p);
  g_color(g, white());
  bd_draw_icon(g, p, icon, bd->flash);
}

void bd_flash(BuildingDisplay *bd) { bd->flash = 1.0f; }

static inline void bd_to_json(CJHObject *o, BuildingDisplay *bd) {
  cjh_o_add_array(o, "location", (CJHWriteArrayCB)ri_to_json, &bd->location);
}

static inline void bd_from_json(CJHObjectR *o, const char *key, BuildingDisplay *bd) {
  if (streq(key, "location")) {
    printf("%.*s%s:\n", indent, space, key);
    indent += 2;
    cjh_o_read_array(o, (CJHReadArrayCB)ri_from_json, &bd->location);
    indent -= 2;

  } else {
    printf("%.*s%s: SKIP\n", indent, space, key);
    cjh_o_skip(o);
  }
}

Vec2 bd_random_point_inside(const BuildingDisplay *bd) {
  Vec2 p = l_to_vecP(ri_bottom_right(bd->location));
  Vec2 s = l_to_vec(bd->location.w - 1, bd->location.h - 1);
  return v_add(p, (Vec2){r_float() * s.x, r_float() * s.y});
}

Vec2 bd_gain_something_location(const BuildingDisplay *bd) {
  Vec2 p = l_to_vecP(ri_bottom_right(bd->location));
  return v_add(p, (Vec2){F * 0.0f, 0.5f * F});
}

#endif