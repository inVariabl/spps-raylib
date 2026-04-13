#ifndef SPAWNER_H
#define SPAWNER_H

#include "objects.h"

// Called once per frame.  Handles periodic spawning and recycling
// of rocks and gold coins relative to the boat's current X position.
void UpdateRockSpawner(GameObject rocks[], int maxRocks,
                       float boatX, int* counter, int separation);

void UpdateGoldSpawner(GameObject gold[], int maxGold,
                       float boatX, int* counter, int separation);

#endif // SPAWNER_H
