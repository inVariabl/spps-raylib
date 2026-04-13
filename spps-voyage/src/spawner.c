#include "spawner.h"
#include "game.h"

// ----------------------------------------------------------------
// Internal helpers
// ----------------------------------------------------------------

// Returns the index of an inactive slot, or the slot whose object
// is furthest ahead of the boat (so we reuse the least-relevant one).
static int FindReuseSlot(GameObject arr[], int count, float boatX)
{
    // Prefer an already-inactive slot
    for (int i = 0; i < count; i++)
        if (!arr[i].active) return i;

    // Fall back to the slot furthest ahead of the boat
    int   slot    = 0;
    float farthest = -1e9f;
    for (int i = 0; i < count; i++)
    {
        float dist = arr[i].position.x - boatX;
        if (dist > farthest) { farthest = dist; slot = i; }
    }
    return slot;
}

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

void UpdateRockSpawner(GameObject rocks[], int maxRocks,
                       float boatX, int* counter, int separation)
{
    (*counter)++;

    // Periodic spawn
    if (*counter % separation == 0)
    {
        int slot = FindReuseSlot(rocks, maxRocks, boatX);
        float rx = boatX - (float)GetRandomValue(20, 100);
        float rz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        rocks[slot].position = (Vector3){ rx, 0.0f, rz };
        rocks[slot].active   = true;
    }

    // Recycle any rock that has drifted behind the boat
    for (int i = 0; i < maxRocks; i++)
    {
        if (rocks[i].active && rocks[i].position.x > boatX + 30.0f)
        {
            float rx = boatX - (float)GetRandomValue(80, 300);
            float rz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
            rocks[i].position = (Vector3){ rx, 0.0f, rz };
        }
    }
}

void UpdateGoldSpawner(GameObject gold[], int maxGold,
                       float boatX, int* counter, int separation)
{
    (*counter)++;

    // Periodic spawn
    if (*counter % separation == 0)
    {
        int slot = FindReuseSlot(gold, maxGold, boatX);
        float gx = boatX - (float)GetRandomValue(20, 100);
        float gz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        gold[slot].position = (Vector3){ gx, 1.5f, gz };
        gold[slot].active   = true;
    }

    // Recycle any coin that has drifted behind the boat
    for (int i = 0; i < maxGold; i++)
    {
        if (gold[i].active && gold[i].position.x > boatX + 30.0f)
        {
            float gx = boatX - (float)GetRandomValue(80, 300);
            float gz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
            gold[i].position = (Vector3){ gx, 1.5f, gz };
        }
    }
}
