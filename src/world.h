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

#define MAX_WATER_REGIONS 8
#define MAX_PORTS 8
#define MAX_LAND_POLY 8

typedef enum {
    WORLD_JUDEA = 0,
    WORLD_MALTA,
    WORLD_PUTEOLI,
    WORLD_NONE
} WorldId;

typedef struct {
    int minX;
    int maxX;
    int minZ;
    int maxZ;
} WaterRegion;

typedef struct {
    Vector3Int position;
    const char *name;
    int nextPortIndex; // -1 if none
    bool active;
} Port;

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
    WaterRegion water[MAX_WATER_REGIONS];
    int waterCount;
    Port ports[MAX_PORTS];
    int portCount;
    WorldId worldId;
    WorldId nextWorldId;
    const char *worldName;
    const char *nextWorldName;
    Vector3Int juliusPos;
    bool hasJulius;
    Vector3Int snakePos;
    bool hasSnake;
    Vector3Int houseArrestPos;
    bool hasHouseArrest;
    Vector2 landPoly[MAX_LAND_POLY];
    int landPolyCount;
    int minX;
    int maxX;
    int minZ;
    int maxZ;
} WorldState;

static const City worldMap[] = {
    {{0, 0, 0}, "Jerusalem", "The Holy City."},
    {{80, 0, 20}, "Sidon", "Port of departure."},
    {{109, 0, 430}, "Malta", "Shipwreck refuge."},
    {{109, 0, 600}, "Puteoli", "Italian port."},
    {{30, 0, 640}, "Forum of Appius", "First welcome on the road."},
    {{20, 0, 680}, "Three Taverns", "Second welcome on the road."},
    {{0, 0, 720}, "Rome", "The Imperial City."}
};

typedef struct {
    WorldState state;
} World;

#include "player.h"

void InitWorld(World *world);
void LoadWorld(World *world, WorldId worldId);
void UpdateWorld(World *world, Player *player);
void DrawWorld(World *world, Camera3D camera);
int GetClickedItem(World *world, Ray ray);
int GetClickedDecoration(World *world, Ray ray);
int GetClickedNPC(World *world, Ray ray);
bool IsTileBlocked(World *world, Vector3Int pos);
bool IsWaterTile(World *world, Vector3Int pos);
int GetPortAt(World *world, Vector3Int pos);
Vector3Int GetGridClicked(Ray ray);
void FindPath(World *world, Player *player, Vector3Int target);

void UpdatePlayer(Player *player, World *world, bool isFirstPerson);
void DrawPlayer(Player *player, Camera3D camera);

#endif // WORLD_H
