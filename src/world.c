#include "world.h"
#include "rlgl.h"
#include <math.h>
#include <stddef.h>
#include <string.h>

static void DrawPalmTree(Vector3 pos) {
    float time = (float)GetTime();
    float windOffset = (pos.x * 1.1f + pos.z * 1.3f); // Unique offset for each tree
    
    // Trunk - slightly curved and segmented
    float trunkHeight = 3.5f;
    int trunkSegments = 5;
    Vector3 currentPos = pos;
    for (int i = 0; i < trunkSegments; i++) {
        float segmentHeight = trunkHeight / trunkSegments;
        float curve = sinf(windOffset + i * 0.5f) * 0.05f;
        Vector3 nextPos = {
            currentPos.x + curve, 
            currentPos.y + segmentHeight, 
            currentPos.z + curve
        };
        float r1 = 0.25f - i * 0.02f;
        float r2 = 0.25f - (i + 1) * 0.02f;
        DrawCylinderEx(currentPos, nextPos, r1, r2, 8, BROWN);
        currentPos = nextPos;
    }

    // Fronds (Leaves)
    int numFronds = 12;
    for (int i = 0; i < numFronds; i++) {
        float angle = i * (360.0f / numFronds);
        float leafWind = sinf(time * 2.0f + windOffset + i) * 0.1f;
        
        rlPushMatrix();
        rlTranslatef(currentPos.x, currentPos.y, currentPos.z);
        rlRotatef(angle + leafWind * 10.0f, 0, 1, 0);
        
        // Draw a curved leaf using segments
        Vector3 leafPos = {0, 0, 0};
        float droop = 0.1f;
        for (int j = 0; j < 6; j++) {
            float segmentLen = 0.5f;
            float windFactor = sinf(time * 3.0f + windOffset + i + j) * 0.02f;
            
            Vector3 nextLeafPos = {
                0, 
                leafPos.y - (droop + windFactor) * j, 
                leafPos.z + segmentLen
            };
            
            Color leafColor = DARKGREEN;
            DrawLine3D(leafPos, nextLeafPos, leafColor);
            
						// Define the vertices once
						Vector3 vLeft  = {leafPos.x - 1.5f / (j + 1), leafPos.y, leafPos.z};
						Vector3 vTip   = {nextLeafPos.x, nextLeafPos.y, nextLeafPos.z};
						Vector3 vRight = {leafPos.x + 1.5f / (j + 1), leafPos.y, leafPos.z};

						// Draw Top Face
						DrawTriangle3D(vLeft, vTip, vRight, leafColor);

						// Draw Bottom Face (Reverse the order of two vertices)
						DrawTriangle3D(vLeft, vRight, vTip, leafColor);

            leafPos = nextLeafPos;
        }

        rlPopMatrix();
    }
}

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
                DrawPalmTree(pos);
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

#define BFS_QUEUE_SIZE 1024

typedef struct {
    Vector3Int pos;
    int parentIdx;
} BFSNode;

void FindPath(World *world, Player *player, Vector3Int finalTarget) {
    if (IsTileBlocked(world, finalTarget)) return;

    // BFS setup
    static BFSNode queue[BFS_QUEUE_SIZE];
    int head = 0, tail = 0;

    queue[tail++] = (BFSNode){player->position, -1};
    
    int foundIdx = -1;
    while (head < tail && tail < BFS_QUEUE_SIZE - 9) {
        BFSNode current = queue[head++];
        
        if (current.pos.x == finalTarget.x && current.pos.z == finalTarget.z) {
            foundIdx = head - 1;
            break;
        }

        // Limit search distance to avoid freezing
        if (tail > 800) break;
        
        // 8 directions (including diagonals)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) continue;
                
                Vector3Int nextPos = {current.pos.x + dx, 0, current.pos.z + dz};
                if (!IsTileBlocked(world, nextPos)) {
                    // Check if already visited in queue
                    bool visited = false;
                    for (int i = 0; i < tail; i++) {
                        if (queue[i].pos.x == nextPos.x && queue[i].pos.z == nextPos.z) {
                            visited = true;
                            break;
                        }
                    }
                    if (!visited) {
                        queue[tail++] = (BFSNode){nextPos, head - 1};
                    }
                }
            }
        }
    }
    
    if (foundIdx != -1) {
        // Trace back path
        int pathIdx = 0;
        int curr = foundIdx;
        static Vector3Int tempPath[MAX_PATH_SIZE];
        while (curr != -1 && pathIdx < MAX_PATH_SIZE) {
            tempPath[pathIdx++] = queue[curr].pos;
            curr = queue[curr].parentIdx;
        }
        
        // Reverse and set in player
        player->pathSize = 0;
        // Skip current tile (tempPath[pathIdx-1])
        for (int i = pathIdx - 2; i >= 0; i--) {
            player->path[player->pathSize++] = tempPath[i];
        }
        player->pathIndex = 0;
        player->finalTarget = finalTarget;
        if (player->pathSize > 0) player->target = player->path[0];
    } else {
        // If no path found (too far or blocked), just set a straight line target for now
        // This keeps it playable even if BFS fails for long distances.
        player->pathSize = 0;
        player->target = finalTarget;
        player->finalTarget = finalTarget;
    }
}
