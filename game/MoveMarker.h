#ifndef MOVEMARKER
#define MOVEMARKER

#include "game/Game.h"
#include "game/GameScene.h"
#include "game/SceneObject.h"
#include "game/assets.h"
#include "gc/gc.h"
#include "math/Rect.h"
#include "math/Vec2.h"

typedef bool (*MoveMarkerContextActiveCB)(void *);
typedef void (*MoveMarkerContextMoveCB)(void *, Vec2);
typedef struct MoveMarker {
  const sg_image *texture;
  Vec2 position;
  bool highlight;

  void *context;
  MoveMarkerContextActiveCB context_active;
  MoveMarkerContextMoveCB context_move;
} MoveMarker;

bool MoveMarker_dead(MoveMarker *mm) { return !mm->context || !mm->context_active || !mm->context_active(mm->context); }

void MoveMarker_update(MoveMarker *mm, Game *g, float dt) {
  (void)mm;
  (void)g;
  (void)dt;
}

void MoveMarker_draw(MoveMarker *mm, Game *g) {
  d_noise(g, 0.0 * (mm->highlight ? 0.5f : 0.2f));
  d_color(g, (Color){mm->highlight ? 1.0f : 0.5f, 0.7f, 0.8f, 1.0f});
  d_object(g, d_animation_buffer(g), mm->texture, mm->position, Game_frame(g) % 4);
}

static bool MoveMarker_mouse_hit(MoveMarker *mm, Vec2 mp) {
  return Rect_contains((Rect){mm->position, (Vec2){16, 16}}, mp);
}

static void MoveMarker_enter(MoveMarker *mm, Game *g) {
  (void)g;
  mm->highlight = true;
}

static void MoveMarker_leave(MoveMarker *mm, Game *g) {
  (void)g;
  mm->highlight = false;
}

static void MoveMarker_activate(MoveMarker *mm, Game *g, GameScene *gs) {
  (void)g;
  (void)gs;
  if (mm->context && mm->context_move)
    mm->context_move(mm->context, mm->position);
}

SceneObjectTable MoveMarker_table = (SceneObjectTable){
    .dead = (SceneObjectDeadCB)MoveMarker_dead,
    .update = (SceneObjectUpdateCB)MoveMarker_update,
    .draw = (SceneObjectDrawCB)MoveMarker_draw,
    .mouse_hit = (SceneObjectMouseHitCB)MoveMarker_mouse_hit,
    .enter = (SceneObjectEnterCB)MoveMarker_enter,
    .leave = (SceneObjectLeaveCB)MoveMarker_leave,
    .activate = (SceneObjectActivateCB)MoveMarker_activate,
};
MoveMarker *MoveMarker_init(Game *g, GameScene *gs, Vec2 pos, void *context, MoveMarkerContextActiveCB active,
                            MoveMarkerContextMoveCB move) {
  MoveMarker *mm = gc_malloc(&gc, sizeof(MoveMarker));
  *mm = (MoveMarker){
      .texture = g_image(g, Img_marker),
      .position = pos,
      .context = context,
      .context_active = active,
      .context_move = move,
  };
  GameScene_add_object(gs, (SceneObject){.context = mm, &MoveMarker_table});
  return mm;
}

#endif