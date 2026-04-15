#include "crew.h"

#define PATROL_MIN  (-INTERIOR_ROOM_D * 0.45f)
#define PATROL_MAX  ( INTERIOR_ROOM_D * 0.45f)

// ----------------------------------------------------------------
// Shared model — loaded once, drawn for every crew member
// ----------------------------------------------------------------
static Model s_model;
static bool  s_modelLoaded = false;

void InitCrewModel(void)
{
    if (!FileExists("spps-voyage/models/roman_character.glb")) return;

    s_model = LoadModel("spps-voyage/models/roman_character.glb");

    if (!IsModelValid(s_model))
    {
        TraceLog(LOG_WARNING, "CREW: Failed to load roman_character.glb");
        return;
    }

    s_modelLoaded = true;
    TraceLog(LOG_INFO, "CREW: roman_character.glb loaded — meshCount=%d materialCount=%d",
             s_model.meshCount, s_model.materialCount);

    BoundingBox bb = GetModelBoundingBox(s_model);
    TraceLog(LOG_INFO, "CREW: BoundingBox min=(%.3f %.3f %.3f) max=(%.3f %.3f %.3f)",
             bb.min.x, bb.min.y, bb.min.z, bb.max.x, bb.max.y, bb.max.z);
}

void UnloadCrewModel(void)
{
    if (s_modelLoaded)
    {
        UnloadModel(s_model);
        s_modelLoaded = false;
    }
}

// ----------------------------------------------------------------
// Crew lifecycle
// ----------------------------------------------------------------

void InitCrew(CrewMember crew[], int count)
{
    for (int i = 0; i < count; i++)
    {
        float xOffset = ((i % 2) == 0) ? -1.5f : 1.5f;
        float zPos    = PATROL_MIN + (float)i / (count - 1) * (PATROL_MAX - PATROL_MIN);

        crew[i].position        = (Vector3){ xOffset, 0.5f, zPos };
        crew[i].morale          = CREW_MORALE_MAX;
        crew[i].patrolTarget    = (i % 2 == 0) ? PATROL_MAX : PATROL_MIN;
        crew[i].walkSpeed       = 1.8f + (float)(i % 3) * 0.4f;
        crew[i].facingPos       = (crew[i].patrolTarget > 0);
        crew[i].boostedRecently = false;
        crew[i].boostTimer      = 0.0f;
    }
}

void UpdateCrew(CrewMember crew[], int count, float dt)
{
    for (int i = 0; i < count; i++)
    {
        CrewMember* c = &crew[i];

        // Drain morale
        c->morale -= CREW_MORALE_DRAIN * dt;
        if (c->morale < 0.0f) c->morale = 0.0f;

        // Patrol movement
        float dir = (c->patrolTarget > c->position.z) ? 1.0f : -1.0f;
        c->position.z += dir * c->walkSpeed * dt;
        c->facingPos = (dir > 0);

        if (c->position.z >= PATROL_MAX) { c->position.z = PATROL_MAX; c->patrolTarget = PATROL_MIN; }
        else if (c->position.z <= PATROL_MIN) { c->position.z = PATROL_MIN; c->patrolTarget = PATROL_MAX; }

        // Boost flash timer
        if (c->boostedRecently)
        {
            c->boostTimer -= dt;
            if (c->boostTimer <= 0.0f) { c->boostedRecently = false; c->boostTimer = 0.0f; }
        }
    }
}

float GetOverallMorale(const CrewMember crew[], int count)
{
    if (count == 0) return CREW_MORALE_MAX;
    float sum = 0.0f;
    for (int i = 0; i < count; i++) sum += crew[i].morale;
    return sum / (float)count;
}

// ----------------------------------------------------------------
// Morale bar — color-coded floating bar above each crew member
// ----------------------------------------------------------------
static void DrawMoraleBar3D(Vector3 basePos, float morale)
{
    float barW = 1.2f;
    float barH = 0.18f;
    float yOff = 2.4f;

    float frac    = morale / CREW_MORALE_MAX;
    Color fillCol = (frac > 0.5f) ? GREEN : (frac > 0.25f) ? ORANGE : RED;
    Color bgCol   = { fillCol.r / 3, fillCol.g / 3, fillCol.b / 3, 255 };

    Vector3 center = { basePos.x, basePos.y + yOff, basePos.z };

    // Full-width background (same color on both sides)
    DrawCube(center, barW, barH, 0.12f, bgCol);

    // Fill — centered so it looks the same from both sides
    float fillW = barW * frac;
    DrawCube(center, fillW, barH, 0.14f, fillCol);
}

// ----------------------------------------------------------------
// Draw crew — static roman_character.glb, no animation
// ----------------------------------------------------------------
void DrawCrew(const CrewMember crew[], int count)
{
    for (int i = 0; i < count; i++)
    {
        const CrewMember* c = &crew[i];

        if (s_modelLoaded)
        {
            float facingAngle = c->facingPos ? 0.0f : 180.0f;
            Color tint = c->boostedRecently ? WHITE : (Color){220, 220, 220, 255};

            DrawModelEx(s_model,
                        c->position,
                        (Vector3){ 0, 1, 0 },
                        facingAngle,
                        (Vector3){ 1.5f, 2.2f, 1.5f },  // tweak scale if model is too big/small
                        tint);
        }
        else
        {
            // Fallback capsule if model failed to load
            Vector3 bodyPos = { c->position.x, 0.6f, c->position.z };
            DrawCylinder(bodyPos, 0.35f, 0.35f, 1.2f, 8, GRAY);
            DrawSphere((Vector3){ c->position.x, 1.95f, c->position.z }, 0.32f, GRAY);
        }

        DrawMoraleBar3D(c->position, c->morale);
    }
}
