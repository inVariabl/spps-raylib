#ifndef NPC_H
#define NPC_H

#include "common.h"

struct World;
struct Player;

typedef struct {
    Vector3Int position;
    Vector3Int homePosition;
    Vector3Int patrolTarget;
    Vector3Int stepTarget;
    const char *name;
    Vector2 facingDirection;
    Vector3 lerpPosition;
    float moveTimer;
    float idleTimer;
    float detectionTimer;
    float attackCooldown;
    float preachCooldown;
    int patrolRadius;
    SpriteType sprite;
    bool active;
    bool patrols;
} NPC;

void InitNPC(NPC *npc, const char *name, Vector3Int position, SpriteType sprite, bool patrols, int patrolRadius);
void UpdateNPC(NPC *npc, struct World *world, struct Player *player, int npcIndex);
void DrawNPC(const NPC *npc, Camera3D camera);

#endif // NPC_H
