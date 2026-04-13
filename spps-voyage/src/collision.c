#include "collision.h"

// ----------------------------------------------------------------
// Internal helper — builds a world-space bounding box for an object.
// ----------------------------------------------------------------
static BoundingBox WorldBox(const GameObject* obj)
{
    BoundingBox box = GetModelBoundingBox(obj->model);
    box.min = Vector3Add(box.min, obj->position);
    box.max = Vector3Add(box.max, obj->position);
    return box;
}

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

int CheckRockCollisions(GameObject* boat, GameObject rocks[], int maxRocks,
                        Sound crash)
{
    int damage = 0;
    BoundingBox boatBox = WorldBox(boat);

    for (int i = 0; i < maxRocks; i++)
    {
        if (!rocks[i].active) continue;

        BoundingBox rockBox = WorldBox(&rocks[i]);
        if (CheckCollisionBoxes(boatBox, rockBox))
        {
            if (IsSoundValid(crash)) PlaySound(crash);
            TraceLog(LOG_INFO, "Boat hit a rock!");
            damage             += 15;  // 15 damage per rock (7 hits to sink)
            rocks[i].active     = false;
        }
    }
    return damage;
}

void CheckGoldCollisions(GameObject* boat, GameObject gold[], int maxGold,
                         Sound goldSound, int* collected)
{
    BoundingBox boatBox = WorldBox(boat);

    for (int i = 0; i < maxGold; i++)
    {
        if (!gold[i].active) continue;

        BoundingBox goldBox = WorldBox(&gold[i]);
        if (CheckCollisionBoxes(boatBox, goldBox))
        {
            if (IsSoundValid(goldSound)) PlaySound(goldSound);
            TraceLog(LOG_INFO, "Boat collected a gold coin!");
            (*collected)++;
            gold[i].active = false;
        }
    }
}
