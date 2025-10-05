#ifndef DUNGEONSCENE_H
#define DUNGEONSCENE_H

#include "engine/interaction/Selectable.h"
#include "engine/math/Rect.h"
#include "engine/scene/SceneObject.h"

Point ds_to_grid(Vec2 p) { return (Point){(int)((p.x + 8) / 16), (int)((p.y + 8) / 16)}; }
Vec2 ds_from_grid(Point s) { return (Vec2){s.x * 16.0f, s.y * 16.0f}; }
Rect ds_rect_from_grid(Point s) { return (Rect){{s.x * 16 - 8, s.y * 16 - 8}, {16, 16}}; }

typedef struct DungeonScene DungeonScene;

DungeonScene *ds_get(Game *g);

int ds_turn(const DungeonScene *ds);

void ds_add_object(DungeonScene *ds, Game *g, SceneObject so);
void ds_set_selectable(DungeonScene *ds, Selectable sl);

#endif