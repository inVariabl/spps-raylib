#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

typedef struct {
    Vector3Int position;
    Vector3Int target;
    Vector3 lerpPosition;
    Slot inventory[INVENTORY_SIZE];
    Skill skills[SKILL_COUNT];
    int spirit; // Conviction/HP
    int maxSpirit;
    int activeQuestId; // 0 = None
    QuestState questStates[10]; // Tracking states for first 10 quests
} Player;

void InitPlayer(Player *player);
void UpdatePlayer(Player *player);
void DrawPlayer(Player *player, Camera3D camera);
bool AddToInventory(Player *player, int itemId);
bool TryCraftTent(Player *player);

#endif // PLAYER_H
