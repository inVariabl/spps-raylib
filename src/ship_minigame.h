#ifndef SHIP_MINIGAME_H
#define SHIP_MINIGAME_H

#include "raylib.h"
#include "world.h"

typedef enum {
    SHIP_MINIGAME_IDLE = 0,
    SHIP_MINIGAME_LOADING,
    SHIP_MINIGAME_PLAYING,
    SHIP_MINIGAME_RETURNING
} ShipMinigamePhase;

typedef struct {
    bool active;
    bool failed;
    bool completed;
    WorldId destinationWorld;
    const char *fromName;
    const char *toName;
    ShipMinigamePhase phase;
    float phaseTimer;
    char resultMessage[160];
} ShipMinigame;

void InitShipMinigame(ShipMinigame *game);
void StartShipMinigame(ShipMinigame *game, WorldId destinationWorld, const char *fromName, const char *toName);
void UpdateShipMinigame(ShipMinigame *game);
void DrawShipMinigame(const ShipMinigame *game, int screenWidth, int screenHeight);
void UnloadShipMinigame(void);

#endif
