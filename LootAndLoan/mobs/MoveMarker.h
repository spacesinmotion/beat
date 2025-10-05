#ifndef MOVE_MARKER_H
#define MOVE_MARKER_H

#include "LootAndLoan/DungeonScene.h"
#include "engine/Game.h"
#include "engine/interaction/Selectable.h"
#include "engine/math/Rect.h"
#include "engine/math/Vec2.h"

void ds_move_player(DungeonScene *ds, Game *g, Point p);

typedef struct MoveMarker {
  Point position;
  float focus;
  int turn;
} MoveMarker;

void sl_mouse_click(MoveMarker *mm, Game *g) { ds_move_player(ds_get(g), g, mm->position); }

static SelectableTable MoveMarger_selectable = {.click = (SelectableMouseClick)sl_mouse_click};

static bool mm_die(MoveMarker *mm, Game *g, bool force) {
  if (force || mm->focus < -1.0f) {
    g_free(g, mm);
    return true;
  }
  return false;
}

static float mm_render_order(const MoveMarker *mm) { return mm->position.y; }

static void mm_update(MoveMarker *mm, Game *g) {
  const float dt = g_animation_delta(g);
  if (ds_turn(ds_get(g)) != mm->turn) {
    mm->focus -= 2.0f * dt;
    return;
  }

  const bool hovered = r_contains(ds_rect_from_grid(mm->position), g_mouse_in_scene(g));
  if (hovered)
    ds_set_selectable(ds_get(g), (Selectable){mm, &MoveMarger_selectable});
  mm->focus += 4.0f * (hovered ? dt : -dt);
  mm->focus = f_clamp(mm->focus, 0.0f, 1.0f);
}

static void mm_draw(const MoveMarker *mm, Game *g) {
  const float m = mm->focus * 50;
  const float x = mm->focus < 0.0 ? 1.0 + mm->focus : 1.0;
  g_color(g, rgba(0x88, 0x97, 0xbd, 50 + m + x * 10 * sin(8 * g_time(g))));
  const Vec2 p = ds_from_grid(mm->position);
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

MoveMarker *MoveMarker_create(DungeonScene *ds, Game *g, Point p, int turn) {
  MoveMarker *mm = g_malloc(g, sizeof(MoveMarker));
  *mm = (MoveMarker){
      .position = p,
      .turn = turn,
      .focus = 0.0f,
  };

  ds_add_object(ds, g, (SceneObject){mm, &MoveMarker_table});

  return mm;
}

#endif // MOVE_MARKER_H
