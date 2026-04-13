#ifndef COLLISION_H
#define COLLISION_H

#include "objects.h"

// Checks boat vs every active rock.  Deducts 20 HP per hit, deactivates
// the rock, and plays the crash sound.  Returns total damage dealt this frame.
int CheckRockCollisions(GameObject* boat, GameObject rocks[], int maxRocks,
                        Sound crash);

// Checks boat vs every active gold coin.  Increments *collected per pickup,
// deactivates the coin, and plays the gold sound.
void CheckGoldCollisions(GameObject* boat, GameObject gold[], int maxGold,
                         Sound goldSound, int* collected);

#endif // COLLISION_H
