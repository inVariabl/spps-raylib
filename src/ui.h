#ifndef UI_H
#define UI_H

#include "player.h"

#include "world.h"

void DrawInventory(Player *player);
void DrawSkills(Player *player);
void DrawHUD(Player *player, World *world, bool isFirstPerson);

#endif // UI_H
