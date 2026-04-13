#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"

#define MAX_PATH_SIZE 256

typedef struct Player {
    Vector3Int position;
    Vector3Int target; // CURRENT step in path
    Vector3Int finalTarget; // Ultimate destination
    Vector3Int path[MAX_PATH_SIZE];
    int pathSize;
    int pathIndex;

    float moveTimer;
    float yaw;
    float pitch;
    Vector3 lerpPosition;
    Slot inventory[INVENTORY_SIZE];
    Skill skills[SKILL_COUNT];
    int spirit; // Conviction/HP
    int maxSpirit;
    int wantedStars;
    float wantedDecayTimer;
    int activeQuestId; // 0 = None
    QuestState questStates[10]; // Tracking states for first 10 quests
    int preachingNpcIndex;
    float preachHoldTimer;
    float preachSuccessTimer;
    char worldMessage[160];
    float worldMessageTimer;

    bool guardDialogueActive;
    int guardDialogueStep;
    int guardDialogueLastResult;
    bool guardClearedForShip;

    bool showInventory;
    bool showMap;
    bool gameComplete;
    bool romeBelieversMet[3];
    bool romeCenturionMet;

} Player;

void InitPlayer(Player *player);
void ResetPlayerMovement(Player *player, Vector3Int position);
void DamagePlayerSpirit(Player *player, int amount);
bool AddToInventory(Player *player, int itemId);
bool TryCraftTent(Player *player);

#endif // PLAYER_H
