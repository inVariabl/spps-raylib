#include "player.h"
#include <stdio.h>

void InitPlayer(Player *player) {
    player->position = (Vector3Int){0, 0, 0};
    player->target = player->position;
    player->lerpPosition = (Vector3){0, 0, 0};
    player->spirit = 100;
    player->maxSpirit = 100;
    player->activeQuestId = 0;
    for (int i = 0; i < 10; i++) player->questStates[i] = QUEST_NOT_STARTED;
    
    // Initial quest available in Jerusalem
    player->questStates[1] = QUEST_NOT_STARTED; // The Antioch Scroll

    for (int i = 0; i < SKILL_COUNT; i++) {
        player->skills[i].level = 1;
        player->skills[i].currentXp = 0;
    }

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        player->inventory[i].itemId = 0;
        player->inventory[i].quantity = 0;
    }
}

void UpdatePlayer(Player *player) {
    // Smooth visual movement (Lerp)
    player->lerpPosition.x = Lerp(player->lerpPosition.x, (float)player->target.x, 0.15f);
    player->lerpPosition.y = 0; // Keeping player on flat ground for now
    player->lerpPosition.z = Lerp(player->lerpPosition.z, (float)player->target.z, 0.15f);

    // Update logical position when close to target
    if (Vector3Distance(player->lerpPosition, (Vector3){(float)player->target.x, (float)player->target.y, (float)player->target.z}) < 0.05f) {
        player->position = player->target;
    }
}

void DrawPlayer(Player *player, Camera3D camera) {
    Vector3 pos = {player->lerpPosition.x, 0.75f, player->lerpPosition.z};
    DrawBillboard(camera, spriteDatabase[SPRITE_PAUL], pos, 1.5f, WHITE);
}

bool AddToInventory(Player *player, int itemId) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].itemId == 0) {
            player->inventory[i].itemId = itemId;
            player->inventory[i].quantity = 1;
            return true;
        }
    }
    return false;
}

bool TryCraftTent(Player *player) {
    int logsIdx = -1;
    int canvasIdx = -1;

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].itemId == 3) logsIdx = i; // Logs
        if (player->inventory[i].itemId == 4) canvasIdx = i; // Canvas
    }

    if (logsIdx != -1 && canvasIdx != -1) {
        player->inventory[logsIdx].itemId = 0;
        player->inventory[canvasIdx].itemId = 0;
        AddToInventory(player, 6); // Tent
        player->skills[SKILL_TENTMAKING].currentXp += 100;
        int xpNeeded = player->skills[SKILL_TENTMAKING].level * 200;
        if (player->skills[SKILL_TENTMAKING].currentXp >= xpNeeded) {
            player->skills[SKILL_TENTMAKING].level++;
            player->skills[SKILL_TENTMAKING].currentXp -= xpNeeded;
        }
        return true;
    }
    return false;
}
