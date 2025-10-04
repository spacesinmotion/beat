#ifndef MOVE_MARKER_H
#define MOVE_MARKER_H

#include "engine/Game.h"
#include "engine/scene/SceneObject.h"

typedef struct DungeonScene DungeonScene;

void ds_add_object(DungeonScene *ds, Game *g, SceneObject so);

typedef struct MoveMarker {
  Vec2 position;
  int distance;
} MoveMarker;

static bool mm_die(MoveMarker *mm, Game *g, bool force) {
  if (force) {
    g_free(g, mm);
    return true;
  }
  return false;
}

static float mm_render_order(const MoveMarker *mm) { return -mm->position.y; }

static void mm_update(MoveMarker *mm, Game *g) {
  (void)mm;
  (void)g;
  // TODO: Implement update logic
}

static void mm_draw(const MoveMarker *mm, Game *g) {
  const Vec2 p = mm->position;
  g_color(g, rgba(0x88, 0x97, 0xbd, 75 + 10 * sin(8 * g_time(g))));
  g_draw_icon(g, Img_menubar, 7, dt_p(p));
}

static void mm_save(CJHObject *o, const MoveMarker *mm) {
  (void)o;
  (void)mm;
  // TODO: Implement save logic
}

static SceneObjectTable MoveMarker_table = {
    .type = "MoveMarker",
    .die = (SceneObjectDieCB)mm_die,
    .render_order = (SceneObjectRenderOrderCB)mm_render_order,
    .update = (SceneObjectUpdateCB)mm_update,
    .draw = (SceneObjectDrawCB)mm_draw,
    .save = (SceneObjectSaveCB)mm_save,
};

MoveMarker *MoveMarker_create(DungeonScene *ds, Game *g, Vec2 pos, int distance) {
  MoveMarker *mm = g_malloc(g, sizeof(MoveMarker));
  *mm = (MoveMarker){.position = pos, .distance = distance};

  ds_add_object(ds, g, (SceneObject){mm, &MoveMarker_table});

  return mm;
}

#endif // MOVE_MARKER_H
