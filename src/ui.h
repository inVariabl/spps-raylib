#ifndef UI_H
#define UI_H

#include "player.h"

#include "world.h"

void DrawInventory(Player *player);
void DrawSkills(Player *player);
void DrawHUD(Player *player, World *world, bool isFirstPerson);
void UpdateGuardDialogue(Player *player, World *world);
void DrawGuardDialogue(Player *player, World *world);
void DrawShaderDebugUI(ShaderSettings *settings);

#endif // UI_H
