#ifndef DELIVERJOB_H
#define DELIVERJOB_H

#include "game/Game.h"
#include "game/assets.h"
#include "math/Color.h"
#include "math/Rect.h"
#include <stdbool.h>

typedef struct GameScene GameScene;

typedef bool (*CollectDoneCB)(void *, GameScene *);
typedef void (*DeliverDoneCB)(void *, GameScene *);
typedef struct DeliverJob {
  void *context;
  CollectDoneCB collect_done;
  DeliverDoneCB deliver_done;
  Recti from, to;
  MenuIcon icon;
  Color color;
} DeliverJob;

DeliverJob *deliver_job(Recti from, Recti to, MenuIcon icon, Color color, void *context, CollectDoneCB on_collect,
                        DeliverDoneCB on_delivered) {
  DeliverJob *job = g_malloc(sizeof(DeliverJob));
  *job = (DeliverJob){context, on_collect, on_delivered, from, to, icon, color};
  return job;
}

bool dj_on_collect(DeliverJob *dj, GameScene *gs) { return !dj->collect_done || dj->collect_done(dj->context, gs); }
void dj_on_delivered(DeliverJob *dj, GameScene *gs) {
  if (dj->deliver_done)
    dj->deliver_done(dj->context, gs);
}

#endif