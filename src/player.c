#include "player.h"
#include <stdio.h>
#include <string.h>
#include "world.h"

void ResetPlayerMovement(Player *player, Vector3Int position) {
    player->position = position;
    player->target = position;
    player->finalTarget = position;
    player->pathSize = 0;
    player->pathIndex = 0;
    player->moveTimer = 0.0f;
    player->lerpPosition = (Vector3){(float)position.x, 0.0f, (float)position.z};
}

void InitPlayer(Player *player) {
    ResetPlayerMovement(player, (Vector3Int){0, 0, 0});
    player->spirit = 100;
    player->maxSpirit = 100;
    player->wantedStars = 0;
    player->wantedDecayTimer = 0.0f;
    player->activeQuestId = 0;

    for (int i = 0; i < 10; i++) player->questStates[i] = QUEST_NOT_STARTED;
    player->questStates[1] = QUEST_NOT_STARTED;

    for (int i = 0; i < SKILL_COUNT; i++) {
        player->skills[i].level = 1;
        player->skills[i].currentXp = 0;
    }

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        player->inventory[i].itemId = 0;
        player->inventory[i].quantity = 0;
    }

    player->preachingNpcIndex = -1;
    player->preachHoldTimer = 0.0f;
    player->preachSuccessTimer = 0.0f;
    player->worldMessage[0] = '\0';
    player->worldMessageTimer = 0.0f;

    player->guardDialogueActive = false;
    player->guardDialogueStep = 0;
    player->guardDialogueLastResult = -1;
    player->guardClearedForShip = false;

    player->showInventory = true;
    player->showMap = true;
    player->gameComplete = false;
    for (int i = 0; i < 3; i++) player->romeBelieversMet[i] = false;
    player->romeCenturionMet = false;
}

void DamagePlayerSpirit(Player *player, int amount) {
    if (amount <= 0) return;

    player->spirit -= amount;
    if (player->spirit > 0) return;

    player->spirit = 20;
    ResetPlayerMovement(player, (Vector3Int){0, 0, 0});
}

void UpdatePlayer(Player *player, World *world, bool isFirstPerson) {
    Vector3Int move = {0, 0, 0};

    float speedMultiplier = 1.0f;
    if (IsKeyDown(KEY_LEFT_SHIFT)) speedMultiplier = 10.0f;

    player->moveTimer -= GetFrameTime() * speedMultiplier;

    if (player->moveTimer <= 0) {
        if (isFirstPerson) {
            float forwardX = sinf(player->yaw);
            float forwardZ = cosf(player->yaw);
            float rightX = sinf(player->yaw - PI/2.0f);
            float rightZ = cosf(player->yaw - PI/2.0f);

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
                move.x = (int)roundf(forwardX);
                move.z = (int)roundf(forwardZ);
            }
            else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
                move.x = -(int)roundf(forwardX);
                move.z = -(int)roundf(forwardZ);
            }

            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
                move.x -= (int)roundf(rightX);
                move.z -= (int)roundf(rightZ);
            }
            else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
                move.x += (int)roundf(rightX);
                move.z += (int)roundf(rightZ);
            }

            if (move.x > 1) move.x = 1;
            if (move.x < -1) move.x = -1;
            if (move.z > 1) move.z = 1;
            if (move.z < -1) move.z = -1;
        } else {
            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) { move.x = -1; move.z = -1; }
            else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) { move.x = 1; move.z = 1; }
            else if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) { move.x = -1; move.z = 1; }
            else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { move.x = 1; move.z = -1; }
        }

        if (move.x != 0 || move.z != 0) {
            Vector3Int nextPos = {player->position.x + move.x, 0, player->position.z + move.z};
            if (!IsTileBlocked(world, nextPos)) {
                player->pathSize = 0;
                player->pathIndex = 0;
                player->target = nextPos;
                player->finalTarget = nextPos;
                player->moveTimer = 0.15f;
            }
        }
    }

    Vector3 targetPos = {(float)player->target.x, 0, (float)player->target.z};

    float baseLerp = isFirstPerson ? 0.3f : 0.15f;
    float lerpSpeed = baseLerp * speedMultiplier;
    if (lerpSpeed > 1.0f) lerpSpeed = 1.0f;

    player->lerpPosition.x = Lerp(player->lerpPosition.x, targetPos.x, lerpSpeed);
    player->lerpPosition.y = 0;
    player->lerpPosition.z = Lerp(player->lerpPosition.z, targetPos.z, lerpSpeed);

    float reachDistance = 0.1f * speedMultiplier;
    if (reachDistance > 0.5f) reachDistance = 0.5f;

    if (Vector3Distance(player->lerpPosition, targetPos) < reachDistance) {
        player->position = player->target;

        if (player->pathIndex < player->pathSize - 1) {
            player->pathIndex++;
            player->target = player->path[player->pathIndex];
        }
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
        if (player->inventory[i].itemId == 3) logsIdx = i;
        if (player->inventory[i].itemId == 4) canvasIdx = i;
    }

    if (logsIdx != -1 && canvasIdx != -1) {
        player->inventory[logsIdx].itemId = 0;
        player->inventory[canvasIdx].itemId = 0;
        AddToInventory(player, 6);
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
