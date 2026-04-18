#ifndef CREW_H
#define CREW_H

#include "game.h"

typedef struct {
    Vector3 position;
    float   morale;
    float   patrolTarget;
    float   walkSpeed;
    bool    facingPos;
    bool    boostedRecently;
    float   boostTimer;
} CrewMember;

// Load/unload shared roman_character.glb — call before/after InitCrew
void  InitCrewModel(void);
void  UnloadCrewModel(void);

void  InitCrew(CrewMember crew[], int count);
void  UpdateCrew(CrewMember crew[], int count, float dt);
float GetOverallMorale(const CrewMember crew[], int count);
void  DrawCrew(const CrewMember crew[], int count, Camera3D camera);

#endif // CREW_H
