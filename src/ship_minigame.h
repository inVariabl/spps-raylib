#ifndef SHIP_MINIGAME_H
#define SHIP_MINIGAME_H

#include "raylib.h"
#include "world.h"

#define SHIP_GAME_OBSTACLE_COUNT 18

typedef struct {
    bool active;
    bool failed;
    bool completed;
    float travelProgress;
    float boatX;
    float bobTimer;
    int hullPoints;
    WorldId destinationWorld;
    const char *fromName;
    const char *toName;
    Vector3 obstaclePos[SHIP_GAME_OBSTACLE_COUNT];
    bool obstacleActive[SHIP_GAME_OBSTACLE_COUNT];
    float obstacleRadius[SHIP_GAME_OBSTACLE_COUNT];
    bool obstacleCoin[SHIP_GAME_OBSTACLE_COUNT];
    float spawnTimer;
} ShipMinigame;

void InitShipMinigame(ShipMinigame *game);
void StartShipMinigame(ShipMinigame *game, WorldId destinationWorld, const char *fromName, const char *toName);
void UpdateShipMinigame(ShipMinigame *game);
void DrawShipMinigame(const ShipMinigame *game, int screenWidth, int screenHeight);
void UnloadShipMinigame(void);

#endif
