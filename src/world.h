#ifndef WORLD_H
#define WORLD_H

#include "common.h"

typedef struct {
    Vector3Int position;
    const char *name;
    bool active;
} NPC;

typedef struct {
    Vector3Int coords;
    const char *name;
    const char *description;
} City;

#define MAX_DECORATIONS 200

typedef struct {
    Vector3Int position;
    DecorationType type;
} Decoration;

typedef struct {
    Vector3Int chunkCoord; // DEPRECATED
    GroundItem items[MAX_GROUND_ITEMS];
    NPC npcs[20];
    Decoration decos[MAX_DECORATIONS];
} WorldState;

static const City worldMap[] = {
    {{0, 0, 0}, "Jerusalem", "The Holy City."},
    {{0, 0, 300}, "Damascus", "The Road to Damascus."},
    {{150, 0, 500}, "Antioch", "The First Christians."},
    {{-300, 0, 800}, "Athens", "The Unknown God."},
    {{-600, 0, 1200}, "Rome", "The Imperial City."}
};

typedef struct {
    WorldState state;
} World;

#include "player.h"

void InitWorld(World *world);
void UpdateWorld(World *world, Player *player);
void DrawWorld(World *world, Camera3D camera);
int GetClickedItem(World *world, Ray ray);
int GetClickedNPC(World *world, Ray ray);
bool IsTileBlocked(World *world, Vector3Int pos);
Vector3Int GetGridClicked(Ray ray);

#endif // WORLD_H
