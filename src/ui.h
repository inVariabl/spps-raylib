#ifndef UI_H
#define UI_H

#include "player.h"

#include "world.h"

void DrawInventory(Player *player, int screenWidth, int screenHeight);
void DrawSkills(Player *player);
void DrawHUD(Player *player, World *world);

#endif // UI_H
