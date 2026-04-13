#include "npc.h"

#include "player.h"
#include "world.h"

#include <stdlib.h>

static bool IsNPCDestinationBlocked(World *world, struct Player *player, int npcIndex, Vector3Int pos) {
    if (IsTileBlocked(world, pos)) return true;

    if (player != NULL && player->position.x == pos.x && player->position.z == pos.z) {
        return true;
    }

    for (int i = 0; i < 20; i++) {
        NPC *other = &world->state.npcs[i];
        if (!other->active || i == npcIndex) continue;
        if (other->position.x == pos.x && other->position.z == pos.z) {
            return true;
        }
    }

    return false;
}

static Vector3Int ChoosePatrolTarget(NPC *npc, World *world, struct Player *player, int npcIndex) {
    for (int attempt = 0; attempt < 12; attempt++) {
        int dx = GetRandomValue(-npc->patrolRadius, npc->patrolRadius);
        int dz = GetRandomValue(-npc->patrolRadius, npc->patrolRadius);
        if (dx == 0 && dz == 0) continue;

        Vector3Int target = {
            npc->homePosition.x + dx,
            0,
            npc->homePosition.z + dz
        };

        if (!IsNPCDestinationBlocked(world, player, npcIndex, target)) {
            return target;
        }
    }

    return npc->position;
}

void InitNPC(NPC *npc, const char *name, Vector3Int position, SpriteType sprite, bool patrols, int patrolRadius) {
    npc->position = position;
    npc->homePosition = position;
    npc->patrolTarget = position;
    npc->stepTarget = position;
    npc->name = name;
    npc->facingDirection = (Vector2){0.0f, 1.0f};
    npc->lerpPosition = (Vector3){(float)position.x, 0.0f, (float)position.z};
    npc->moveTimer = 0.0f;
    npc->idleTimer = (float)GetRandomValue(20, 70) / 100.0f;
    npc->detectionTimer = 0.0f;
    npc->attackCooldown = 0.0f;
    npc->preachCooldown = 0.0f;
    npc->patrolRadius = patrolRadius;
    npc->sprite = sprite;
    npc->active = true;
    npc->patrols = patrols;
}

void UpdateNPC(NPC *npc, World *world, struct Player *player, int npcIndex) {
    if (!npc->active) return;

    Vector3 targetPos = {
        (float)npc->stepTarget.x,
        0.0f,
        (float)npc->stepTarget.z
    };
    npc->lerpPosition.x = Lerp(npc->lerpPosition.x, targetPos.x, 0.06f);
    npc->lerpPosition.y = 0.0f;
    npc->lerpPosition.z = Lerp(npc->lerpPosition.z, targetPos.z, 0.06f);

    if (Vector3Distance(npc->lerpPosition, targetPos) < 0.08f) {
        npc->position = npc->stepTarget;
    }

    if (player != NULL &&
        world->state.worldId == WORLD_MALTA &&
        player->preachingNpcIndex == npcIndex &&
        IsKeyDown(KEY_E)) {
        npc->patrolTarget = npc->position;
        npc->stepTarget = npc->position;
        npc->lerpPosition = (Vector3){(float)npc->position.x, 0.0f, (float)npc->position.z};
        npc->moveTimer = 0.0f;
        npc->idleTimer = 0.25f;
        return;
    }

    if (TextIsEqual(npc->name, "Pharisee") && player != NULL && player->wantedStars >= 1) {
        float dt = GetFrameTime();
        npc->moveTimer -= dt;

        Vector2 toPlayer = {
            player->lerpPosition.x - npc->lerpPosition.x,
            player->lerpPosition.z - npc->lerpPosition.z
        };
        float distance = Vector2Length(toPlayer);
        if (distance > 0.25f) {
            npc->facingDirection = Vector2Normalize(toPlayer);
        }

        if (distance <= 1.4f || npc->moveTimer > 0.0f) return;

        int stepX = 0;
        int stepZ = 0;
        if (fabsf(toPlayer.x) >= 0.4f) stepX = (toPlayer.x > 0.0f) ? 1 : -1;
        if (fabsf(toPlayer.y) >= 0.4f) stepZ = (toPlayer.y > 0.0f) ? 1 : -1;

        Vector3Int candidateSteps[3] = {
            {npc->position.x + stepX, 0, npc->position.z + stepZ},
            {npc->position.x + stepX, 0, npc->position.z},
            {npc->position.x, 0, npc->position.z + stepZ}
        };

        for (int i = 0; i < 3; i++) {
            Vector3Int nextStep = candidateSteps[i];
            if ((nextStep.x == npc->position.x && nextStep.z == npc->position.z) ||
                IsNPCDestinationBlocked(world, player, npcIndex, nextStep)) {
                continue;
            }

            Vector2 stepDir = {
                (float)(nextStep.x - npc->position.x),
                (float)(nextStep.z - npc->position.z)
            };
            if (Vector2Length(stepDir) > 0.0f) {
                npc->facingDirection = Vector2Normalize(stepDir);
            }
            npc->stepTarget = nextStep;
            npc->moveTimer = 0.9f;
            return;
        }

        npc->moveTimer = 0.45f;
        return;
    }

    if (!npc->patrols) return;

    float dt = GetFrameTime();
    npc->moveTimer -= dt;
    npc->idleTimer -= dt;
    npc->preachCooldown -= dt;
    if (npc->preachCooldown < 0.0f) npc->preachCooldown = 0.0f;

    if (npc->position.x != npc->patrolTarget.x || npc->position.z != npc->patrolTarget.z) {
        if (npc->moveTimer > 0.0f) return;

        int stepX = npc->patrolTarget.x - npc->position.x;
        int stepZ = npc->patrolTarget.z - npc->position.z;
        if (stepX != 0) stepX /= abs(stepX);
        if (stepZ != 0) stepZ /= abs(stepZ);

        Vector3Int nextStep = {
            npc->position.x + stepX,
            0,
            npc->position.z + stepZ
        };

        if (IsNPCDestinationBlocked(world, player, npcIndex, nextStep)) {
            npc->patrolTarget = ChoosePatrolTarget(npc, world, player, npcIndex);
            npc->stepTarget = npc->position;
            npc->idleTimer = (float)GetRandomValue(25, 80) / 100.0f;
            return;
        }

        npc->facingDirection = Vector2Normalize((Vector2){(float)stepX, (float)stepZ});
        npc->stepTarget = nextStep;
        npc->moveTimer = 0.56f;
        return;
    }

    if (npc->idleTimer > 0.0f) return;

    npc->patrolTarget = ChoosePatrolTarget(npc, world, player, npcIndex);
    npc->stepTarget = npc->position;
    npc->idleTimer = (float)GetRandomValue(60, 180) / 100.0f;
}

void DrawNPC(const NPC *npc, Camera3D camera) {
    if (!npc->active) return;

    DrawBillboard(
        camera,
        spriteDatabase[npc->sprite],
        (Vector3){npc->lerpPosition.x, 0.75f, npc->lerpPosition.z},
        1.5f,
        WHITE
    );
}
