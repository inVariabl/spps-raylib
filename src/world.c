#include "world.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

void InitWorld(World *world) {
    // Clear everything
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) world->state.items[i].active = false;
    for (int i = 0; i < 20; i++) world->state.npcs[i].active = false;
    for (int i = 0; i < MAX_DECORATIONS; i++) world->state.decos[i].type = DECO_NONE;
    
    // Seed Jerusalem (0,0,0)
    world->state.npcs[0] = (NPC){(Vector3Int){2, 0, -3}, "Sadducee", true};
    world->state.items[0] = (GroundItem){(Vector3Int){-3, 0, 4}, 5, true}; // Scroll
    
    // Jerusalem Architecture
    world->state.decos[0] = (Decoration){(Vector3Int){0, 0, -10}, DECO_TEMPLE};
    world->state.decos[1] = (Decoration){(Vector3Int){-15, 0, -5}, DECO_SYNAGOGUE};
    world->state.decos[2] = (Decoration){(Vector3Int){10, 0, 5}, DECO_MARKET};
    world->state.decos[3] = (Decoration){(Vector3Int){5, 0, 10}, DECO_HOUSE};
    world->state.decos[4] = (Decoration){(Vector3Int){-5, 0, 10}, DECO_HOUSE};

    // Damascus (0, 0, 300)
    world->state.npcs[1] = (NPC){(Vector3Int){0, 0, 305}, "Ananias", true};
    world->state.decos[5] = (Decoration){(Vector3Int){10, 0, 305}, DECO_SYNAGOGUE};
    world->state.decos[6] = (Decoration){(Vector3Int){-10, 0, 300}, DECO_HOUSE};
    world->state.decos[7] = (Decoration){(Vector3Int){0, 0, 290}, DECO_MARKET};

    // Random decorations along the path
    for (int i = 10; i < 50; i++) {
        world->state.decos[i].type = (i % 2 == 0) ? DECO_PALM_TREE : DECO_ROCK;
        world->state.decos[i].position = (Vector3Int){GetRandomValue(-20, 20), 0, GetRandomValue(10, 290)};
    }
}

void UpdateWorld(World *world, Player *player) {
}

void DrawWorld(World *world, Camera3D camera) {
    // 1. Tiled Floor (Sand-Colored)
    Vector3 floorPos = {roundf(camera.target.x), -0.01f, roundf(camera.target.z)};
    DrawPlane(floorPos, (Vector2){200, 200}, BEIGE);
    DrawGrid(100, TILE_SIZE);

    // 2. Road System
    for (int z = 0; z < 1500; z += 2) {
        Vector3 roadPos = {0, 0.01f, (float)z};
        if (z > 300) {
            float t = (float)(z - 300) / 200.0f;
            if (t > 1.0f) t = 1.0f;
            roadPos.x = Lerp(0, 150, t);
        }
        float dist = Vector3Distance(camera.target, roadPos);
        if (dist < RENDER_DISTANCE) DrawCube(roadPos, 1.5f, 0.05f, 1.5f, Fade(DARKBROWN, 0.5f));
    }

    // 3. Decorations (Trees, Rocks, Houses, etc.)
    for (int i = 0; i < MAX_DECORATIONS; i++) {
        if (world->state.decos[i].type == DECO_NONE) continue;
        
        Vector3 pos = {(float)world->state.decos[i].position.x, 0, (float)world->state.decos[i].position.z};
        float dist = Vector3Distance(camera.target, pos);
        if (dist > RENDER_DISTANCE + 20) continue;

        switch (world->state.decos[i].type) {
            case DECO_PALM_TREE:
                DrawCylinder(pos, 0.15f, 0.15f, 2.5f, 6, BROWN);
                DrawSphere((Vector3){pos.x, pos.y + 2.5f, pos.z}, 1.0f, DARKGREEN);
                break;
            case DECO_ROCK:
                DrawCube(pos, 1.2f, 0.8f, 1.0f, DARKGRAY);
                break;
            case DECO_HOUSE:
                DrawCube((Vector3){pos.x, 1.5f, pos.z}, 4, 3, 4, LIGHTGRAY);
                DrawCube((Vector3){pos.x, 3.1f, pos.z}, 4.2f, 0.2f, 4.2f, DARKGRAY); // Roof
                break;
            case DECO_SYNAGOGUE:
                DrawCube((Vector3){pos.x, 2.0f, pos.z}, 6, 4, 6, WHITE); // Main Hall
                DrawCube((Vector3){pos.x, 4.2f, pos.z}, 4, 1, 4, LIGHTGRAY); // Upper block
                DrawCube((Vector3){pos.x, 2, pos.z + 3.1f}, 1.5f, 2.5f, 0.5f, BROWN); // Door
                break;
            case DECO_TEMPLE:
                DrawCube((Vector3){pos.x, 2.5f, pos.z}, 10, 5, 12, GOLD); // The Holy Place
                // Pillared porch
                for (int p = -4; p <= 4; p += 2) {
                    DrawCylinder((Vector3){pos.x + p, 0, pos.z + 6.5f}, 0.3f, 0.3f, 5.0f, 6, WHITE);
                }
                DrawCube((Vector3){pos.x, 5.5f, pos.z}, 11, 1, 14, WHITE); // Roof
                break;
            case DECO_MARKET:
                DrawCube((Vector3){pos.x, 0.5f, pos.z}, 6, 1, 6, DARKGRAY); // Platform
                // Stalls
                DrawCube((Vector3){pos.x - 2, 1, pos.z - 2}, 1, 1, 1, ORANGE);
                DrawCube((Vector3){pos.x + 2, 1, pos.z + 2}, 1, 1, 1, BLUE);
                break;
            default: break;
        }
    }

    // 4. Ground Items & 5. NPCs
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (!world->state.items[i].active) continue;
        if (Vector3Distance(camera.target, (Vector3){(float)world->state.items[i].position.x, 0, (float)world->state.items[i].position.z}) < RENDER_DISTANCE) {
            DrawCube((Vector3){(float)world->state.items[i].position.x, 0.15f, (float)world->state.items[i].position.z}, 0.3f, 0.3f, 0.3f, itemDatabase[world->state.items[i].itemId].color);
        }
    }
    for (int i = 0; i < 20; i++) {
        if (!world->state.npcs[i].active) continue;
        if (Vector3Distance(camera.target, (Vector3){(float)world->state.npcs[i].position.x, 0, (float)world->state.npcs[i].position.z}) < RENDER_DISTANCE) {
            Texture2D tex = spriteDatabase[SPRITE_PAUL];
            if (TextIsEqual(world->state.npcs[i].name, "Sadducee")) tex = spriteDatabase[SPRITE_SADDUCEE];
            else if (TextIsEqual(world->state.npcs[i].name, "Ananias")) tex = spriteDatabase[SPRITE_ANANIAS];
            DrawBillboard(camera, tex, (Vector3){(float)world->state.npcs[i].position.x, 0.75f, (float)world->state.npcs[i].position.z}, 1.5f, WHITE);
        }
    }
}

bool IsTileBlocked(World *world, Vector3Int pos) {
    for (int i = 0; i < MAX_DECORATIONS; i++) {
        if (world->state.decos[i].type == DECO_NONE) continue;
        
        // Simple bounding box check based on decoration type
        int halfSize = 1;
        if (world->state.decos[i].type == DECO_SYNAGOGUE) halfSize = 3;
        else if (world->state.decos[i].type == DECO_TEMPLE) halfSize = 6;
        else if (world->state.decos[i].type == DECO_MARKET) halfSize = 3;
        else if (world->state.decos[i].type == DECO_HOUSE) halfSize = 2;
        
        if (pos.x >= world->state.decos[i].position.x - halfSize && 
            pos.x <= world->state.decos[i].position.x + halfSize &&
            pos.z >= world->state.decos[i].position.z - halfSize &&
            pos.z <= world->state.decos[i].position.z + halfSize) {
            return true;
        }
    }
    return false;
}

int GetClickedItem(World *world, Ray ray) {
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (!world->state.items[i].active) continue;
        BoundingBox box = {(Vector3){(float)world->state.items[i].position.x - 0.4f, 0, (float)world->state.items[i].position.z - 0.4f},
                           (Vector3){(float)world->state.items[i].position.x + 0.4f, 0.5f, (float)world->state.items[i].position.z + 0.4f}};
        if (GetRayCollisionBox(ray, box).hit) return i;
    }
    return -1;
}

int GetClickedNPC(World *world, Ray ray) {
    for (int i = 0; i < 20; i++) {
        if (!world->state.npcs[i].active) continue;
        BoundingBox box = {(Vector3){(float)world->state.npcs[i].position.x - 0.4f, 0, (float)world->state.npcs[i].position.z - 0.4f},
                           (Vector3){(float)world->state.npcs[i].position.x + 0.4f, 1.2f, (float)world->state.npcs[i].position.z + 0.4f}};
        if (GetRayCollisionBox(ray, box).hit) return i;
    }
    return -1;
}

Vector3Int GetGridClicked(Ray ray) {
    RayCollision groundHit = GetRayCollisionQuad(
        ray, (Vector3){-5000, 0, -5000}, (Vector3){-5000, 0, 5000},
        (Vector3){5000, 0, 5000}, (Vector3){5000, 0, -5000});
    if (groundHit.hit) return (Vector3Int){(int)roundf(groundHit.point.x), 0, (int)roundf(groundHit.point.z)};
    return (Vector3Int){0, -1, 0};
}
