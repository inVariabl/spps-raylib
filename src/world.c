#include "world.h"
#include "rlgl.h"
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void DrawGroundItemModel(Camera3D camera, Vector3 pos, int itemId) {
    switch (itemId) {
        case 4: // Envelope/mailer style pickup
            DrawCube((Vector3){pos.x, 0.08f, pos.z}, 0.45f, 0.06f, 0.34f, (Color){232, 214, 174, 255});
            DrawLine3D((Vector3){pos.x - 0.22f, 0.12f, pos.z - 0.17f},
                       (Vector3){pos.x, 0.12f, pos.z},
                       DARKBROWN);
            DrawLine3D((Vector3){pos.x + 0.22f, 0.12f, pos.z - 0.17f},
                       (Vector3){pos.x, 0.12f, pos.z},
                       DARKBROWN);
            break;
        case 5: // Floating letter sprite
        {
            float t = (float)GetTime();
            Vector3 letterPos = {
                pos.x + sinf(t * 1.8f + pos.x * 0.4f) * 0.08f,
                0.65f + sinf(t * 2.6f + pos.z * 0.3f) * 0.12f,
                pos.z
            };
            float scale = 0.9f + sinf(t * 3.2f) * 0.08f;
            DrawCircle3D((Vector3){pos.x, 0.03f, pos.z}, 0.22f, (Vector3){1.0f, 0.0f, 0.0f}, 90.0f, Fade(BLACK, 0.25f));
            DrawBillboard(camera, spriteDatabase[SPRITE_LETTER], letterPos, scale, WHITE);
        }
            break;
        default:
            DrawCube((Vector3){pos.x, 0.15f, pos.z}, 0.3f, 0.3f, 0.3f, itemDatabase[itemId].color);
            break;
    }
}

static Vector3 GetShadowOffset(float height) {
    return (Vector3){height * 0.22f, 0.02f, height * 0.14f};
}

static void DrawShadowDisc(Vector3 pos, float radius, Color color) {
    DrawCircle3D((Vector3){pos.x, 0.02f, pos.z}, radius, (Vector3){1.0f, 0.0f, 0.0f}, 90.0f, color);
}

static void DrawDecorationShadow(Decoration deco) {
    Vector3 base = {(float)deco.position.x, 0.0f, (float)deco.position.z};
    Color shadowColor = Fade(BLACK, 0.10f);

    switch (deco.type) {
        case DECO_PALM_TREE:
        {
            Vector3 trunkShadow = Vector3Add(base, GetShadowOffset(2.0f));
            DrawShadowDisc(trunkShadow, 0.26f, Fade(BLACK, 0.10f));

            Vector3 frond1 = Vector3Add(base, GetShadowOffset(3.3f));
            Vector3 frond2 = Vector3Add(base, (Vector3){0.95f, 0.02f, 0.25f});
            Vector3 frond3 = Vector3Add(base, (Vector3){0.55f, 0.02f, 0.85f});
            DrawShadowDisc(frond1, 0.95f, Fade(BLACK, 0.09f));
            DrawShadowDisc(frond2, 0.65f, Fade(BLACK, 0.07f));
            DrawShadowDisc(frond3, 0.58f, Fade(BLACK, 0.07f));
        }
            break;
        case DECO_HOUSE:
            DrawShadowDisc(Vector3Add(base, GetShadowOffset(2.2f)), 1.7f, shadowColor);
            break;
        case DECO_SYNAGOGUE:
            DrawShadowDisc(Vector3Add(base, GetShadowOffset(3.0f)), 2.2f, shadowColor);
            break;
        case DECO_TEMPLE:
            DrawShadowDisc(Vector3Add(base, GetShadowOffset(4.2f)), 3.6f, Fade(BLACK, 0.12f));
            break;
        case DECO_COLUMN:
            DrawShadowDisc(Vector3Add(base, GetShadowOffset(2.6f)), 0.30f, Fade(BLACK, 0.10f));
            break;
        case DECO_FORUM_ARCH:
            DrawShadowDisc(Vector3Add(base, GetShadowOffset(2.2f)), 1.3f, shadowColor);
            break;
        case DECO_SHIP:
            DrawShadowDisc(Vector3Add(base, GetShadowOffset(1.4f)), 1.8f, Fade(BLACK, 0.10f));
            break;
        default:
            break;
    }
}

static void DrawWaterRegion(World *world, WaterRegion r, float time) {
    float width = (float)(r.maxX - r.minX + 1);
    float depth = (float)(r.maxZ - r.minZ + 1);
    float centerX = (r.minX + r.maxX) / 2.0f;
    float centerZ = (r.minZ + r.maxZ) / 2.0f;
    float surge = sinf(time * 1.4f + centerZ * 0.03f) * 0.03f;
    Vector3 waterPos = {centerX, 0.03f + surge, centerZ};

    DrawCubeV(waterPos, (Vector3){width, 0.16f, depth}, (Color){45, 110, 185, 255});

    // Repeating surface bands to fake wave motion without a full mesh system.
    for (int band = 0; band < 5; band++) {
        float zOffset = sinf(time * 1.7f + band * 0.8f) * 2.5f;
        float bandZ = centerZ - depth * 0.35f + band * (depth * 0.18f) + zOffset;
        DrawCubeV((Vector3){centerX, 0.12f + surge * 0.7f, bandZ},
                  (Vector3){width, 0.01f, 1.2f},
                  (Color){80, 135, 198, 255});
    }

    // If the water starts inside the playable map, dress that edge as shoreline.
    if (r.minX <= world->state.maxX && r.minX >= world->state.minX) {
        float shoreStartZ = (float)(r.minZ > world->state.minZ ? r.minZ : world->state.minZ);
        float shoreEndZ = (float)(r.maxZ < world->state.maxZ ? r.maxZ : world->state.maxZ);
        float shoreDepth = shoreEndZ - shoreStartZ + 1.0f;

        if (shoreDepth > 0.0f) {
            float shoreCenterZ = (shoreStartZ + shoreEndZ) / 2.0f;
            float wash = 0.35f + 0.25f * (0.5f + 0.5f * sinf(time * 1.8f));
            float washX = r.minX + 0.35f + wash;
            float shoreLift = 0.03f + 0.03f * sinf(time * 2.8f);

            // Narrow moving shore-break band. This keeps the sea blue while still animating the edge.
            DrawCubeV((Vector3){washX, 0.11f + shoreLift, shoreCenterZ},
                      (Vector3){0.7f, 0.015f, shoreDepth},
                      Fade((Color){170, 220, 235, 255}, 0.30f));

            for (int i = 0; i < 22; i++) {
                float t = (float)i / 21.0f;
                float foamZ = shoreStartZ + t * (shoreEndZ - shoreStartZ);
                float foamAdvance = 0.18f + 0.45f * (0.5f + 0.5f * sinf(time * 2.1f + foamZ * 0.08f));
                float foamY = 0.035f + 0.035f * sinf(time * 3.4f + i * 0.6f);
                float foamRadius = 0.12f + 0.05f * sinf(time * 2.7f + i);
                DrawCircle3D((Vector3){r.minX + foamAdvance, foamY, foamZ},
                             foamRadius,
                             (Vector3){1.0f, 0.0f, 0.0f},
                             90.0f,
                             Fade(RAYWHITE, 0.52f));
            }
        }
    }
}

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

static void DrawRoadSegment(Camera3D camera, World *world, Vector3Int a, Vector3Int b) {
    Vector3 start = {(float)a.x, 0.01f, (float)a.z};
    Vector3 end = {(float)b.x, 0.01f, (float)b.z};
    float dist = Vector3Distance(start, end);
    int steps = (int)(dist / 2.0f);
    if (steps < 1) steps = 1;
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / (float)steps;
        Vector3 pos = Vector3Lerp(start, end, t);
        Vector3Int tile = {(int)roundf(pos.x), 0, (int)roundf(pos.z)};
        if (IsWaterTile(world, tile)) continue;
        float camDist = Vector3Distance(camera.target, pos);
        if (camDist < RENDER_DISTANCE) {
            DrawCube(pos, 1.5f, 0.05f, 1.5f, Fade(DARKBROWN, 0.6f));
        }
    }
}

static void ClearWorldState(World *world) {
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) world->state.items[i].active = false;
    for (int i = 0; i < 20; i++) world->state.npcs[i].active = false;
    for (int i = 0; i < MAX_DECORATIONS; i++) world->state.decos[i].type = DECO_NONE;
    world->state.waterCount = 0;
    world->state.portCount = 0;
    world->state.hasJulius = false;
    world->state.hasSnake = false;
    world->state.hasHouseArrest = false;
    world->state.landPolyCount = 0;
}

static void AddPalmCluster(World *world, int startIdx, int count, int minX, int maxX, int minZ, int maxZ) {
    int idx = startIdx;
    for (int i = 0; i < count && idx < MAX_DECORATIONS; i++, idx++) {
        world->state.decos[idx].type = DECO_PALM_TREE;
        world->state.decos[idx].position = (Vector3Int){GetRandomValue(minX, maxX), 0, GetRandomValue(minZ, maxZ)};
    }
}

void LoadWorld(World *world, WorldId worldId) {
    ClearWorldState(world);
    world->state.worldId = worldId;
    world->state.nextWorldId = WORLD_NONE;
    world->state.worldName = "Unknown";
    world->state.nextWorldName = "Rome";

    switch (worldId) {
        case WORLD_JUDEA:
            world->state.worldName = "Judea";
            world->state.nextWorldName = "Malta";
            world->state.nextWorldId = WORLD_MALTA;
            world->state.minX = -40;
            world->state.maxX = 140;
            world->state.minZ = -40;
            world->state.maxZ = 120;
            world->state.npcs[0] = (NPC){(Vector3Int){2, 0, -3}, "Sadducee", true};
            world->state.items[0] = (GroundItem){(Vector3Int){-3, 0, 4}, 5, true};
            world->state.decos[0] = (Decoration){(Vector3Int){0, 0, -10}, DECO_TEMPLE};
            world->state.decos[1] = (Decoration){(Vector3Int){-15, 0, -5}, DECO_SYNAGOGUE};
            world->state.decos[2] = (Decoration){(Vector3Int){10, 0, 5}, DECO_HOUSE};
            world->state.decos[3] = (Decoration){(Vector3Int){5, 0, 10}, DECO_HOUSE};
            world->state.decos[4] = (Decoration){(Vector3Int){-5, 0, 10}, DECO_HOUSE};
            AddPalmCluster(world, 10, 20, -20, 20, 5, 80);
            // Extend the sea well beyond the playable edge so Judea reads like a shoreline, not a boxed-in pond.
            world->state.water[0] = (WaterRegion){81, 320, -80, 80};
            world->state.waterCount = 1;
            world->state.ports[0] = (Port){(Vector3Int){80, 0, 20}, "Sidon", 0, true};
            world->state.portCount = 1;
            world->state.decos[30] = (Decoration){(Vector3Int){88, 0, 16}, DECO_SHIP};
            world->state.juliusPos = (Vector3Int){78, 0, 22};
            world->state.hasJulius = true;
            break;
        case WORLD_MALTA:
            world->state.worldName = "Malta";
            world->state.nextWorldName = "Puteoli";
            world->state.nextWorldId = WORLD_PUTEOLI;
            world->state.minX = 70;
            world->state.maxX = 150;
            world->state.minZ = 360;
            world->state.maxZ = 460;
            // Sea around the island (leave center for land)
            world->state.water[0] = (WaterRegion){70, 150, 360, 380};   // south band
            world->state.water[1] = (WaterRegion){70, 150, 440, 460};   // north band
            world->state.water[2] = (WaterRegion){70, 90, 380, 440};    // west band
            world->state.water[3] = (WaterRegion){130, 150, 380, 440};  // east band
            world->state.waterCount = 4;
            world->state.landPolyCount = 6;
            world->state.landPoly[0] = (Vector2){90, 385};
            world->state.landPoly[1] = (Vector2){130, 390};
            world->state.landPoly[2] = (Vector2){140, 415};
            world->state.landPoly[3] = (Vector2){125, 440};
            world->state.landPoly[4] = (Vector2){95, 435};
            world->state.landPoly[5] = (Vector2){85, 405};
            AddPalmCluster(world, 0, 15, 80, 140, 370, 450);
            world->state.decos[20] = (Decoration){(Vector3Int){120, 0, 410}, DECO_ROCK};
            world->state.decos[21] = (Decoration){(Vector3Int){100, 0, 420}, DECO_ROCK};
            world->state.decos[22] = (Decoration){(Vector3Int){109, 0, 418}, DECO_FIRE_PIT_UNLIT};
            // Snake hidden initially (will appear when fire is lit)
            world->state.decos[23] = (Decoration){(Vector3Int){109, 0, 418}, DECO_NONE};
            world->state.npcs[0] = (NPC){(Vector3Int){112, 0, 420}, "Islander", true};
            world->state.npcs[1] = (NPC){(Vector3Int){106, 0, 415}, "Islander", true};
            world->state.ports[0] = (Port){(Vector3Int){109, 0, 430}, "Malta", 0, true};
            world->state.portCount = 1;
            world->state.snakePos = (Vector3Int){109, 0, 418};
            world->state.hasSnake = true;
            break;
        case WORLD_PUTEOLI:
            world->state.worldName = "Puteoli";
            world->state.nextWorldName = "Rome";
            world->state.nextWorldId = WORLD_NONE;
            world->state.minX = -20;
            world->state.maxX = 140;
            world->state.minZ = 560;
            world->state.maxZ = 760;
            world->state.decos[0] = (Decoration){(Vector3Int){100, 0, 600}, DECO_HOUSE};
            world->state.decos[1] = (Decoration){(Vector3Int){90, 0, 610}, DECO_HOUSE};
            world->state.decos[2] = (Decoration){(Vector3Int){70, 0, 660}, DECO_FORUM_ARCH};
            world->state.decos[3] = (Decoration){(Vector3Int){40, 0, 650}, DECO_COLUMN};
            world->state.decos[4] = (Decoration){(Vector3Int){30, 0, 650}, DECO_COLUMN};
            world->state.decos[5] = (Decoration){(Vector3Int){20, 0, 650}, DECO_COLUMN};
            world->state.decos[6] = (Decoration){(Vector3Int){40, 0, 670}, DECO_COLUMN};
            world->state.decos[7] = (Decoration){(Vector3Int){30, 0, 670}, DECO_COLUMN};
            world->state.decos[8] = (Decoration){(Vector3Int){20, 0, 670}, DECO_COLUMN};
            world->state.decos[9] = (Decoration){(Vector3Int){30, 0, 690}, DECO_TEMPLE};
            world->state.decos[10] = (Decoration){(Vector3Int){0, 0, 720}, DECO_HOUSE};
            world->state.ports[0] = (Port){(Vector3Int){109, 0, 560}, "Puteoli", 0, true};
            world->state.portCount = 1;
            world->state.houseArrestPos = (Vector3Int){0, 0, 720};
            world->state.hasHouseArrest = true;
            break;
        default:
            break;
    }
}

void InitWorld(World *world) {
    LoadWorld(world, WORLD_JUDEA);
}

void UpdateWorld(World *world, Player *player) {
}

void DrawWorld(World *world, Camera3D camera, bool drawShadows) {
    float time = (float)GetTime();

    // 1. Tiled Floor (Sand-Colored)
    float centerX = (world->state.minX + world->state.maxX) / 2.0f;
    float centerZ = (world->state.minZ + world->state.maxZ) / 2.0f;
    float sizeX = (float)(world->state.maxX - world->state.minX);
    float sizeZ = (float)(world->state.maxZ - world->state.minZ);
    Vector3 floorPos = {centerX, -0.01f, centerZ};
    if (world->state.landPolyCount > 2) {
        DrawPlane(floorPos, (Vector2){sizeX, sizeZ}, BEIGE);
    } else {
        DrawPlane(floorPos, (Vector2){sizeX, sizeZ}, BEIGE);
        int gridCount = (int)(fmaxf(sizeX, sizeZ) / 2.0f);
        if (gridCount < 10) gridCount = 10;
        DrawGrid(gridCount, TILE_SIZE);
    }

    // 1.5 Water regions
    for (int i = 0; i < world->state.waterCount; i++) {
        DrawWaterRegion(world, world->state.water[i], time);
    }

    if (world->state.landPolyCount > 2) {
        Vector2 *poly = world->state.landPoly;
        for (int i = 1; i < world->state.landPolyCount - 1; i++) {
            Vector3 a = {poly[0].x, -0.01f, poly[0].y};
            Vector3 b = {poly[i].x, -0.01f, poly[i].y};
            Vector3 c = {poly[i + 1].x, -0.01f, poly[i + 1].y};
            DrawTriangle3D(a, b, c, BEIGE);
            DrawTriangle3D(a, c, b, BEIGE);
        }
    }

    // 2. Road System (guidance path)
    if (world->state.worldId == WORLD_JUDEA) {
        DrawRoadSegment(camera, world, (Vector3Int){0, 0, 0}, world->state.ports[0].position);
    } else if (world->state.worldId == WORLD_PUTEOLI) {
        DrawRoadSegment(camera, world, world->state.ports[0].position, (Vector3Int){60, 0, 600});
        DrawRoadSegment(camera, world, (Vector3Int){60, 0, 600}, (Vector3Int){30, 0, 640});
        DrawRoadSegment(camera, world, (Vector3Int){30, 0, 640}, (Vector3Int){20, 0, 680});
        DrawRoadSegment(camera, world, (Vector3Int){20, 0, 680}, (Vector3Int){0, 0, 720});
    }

    // 3. Decorations (Trees, Rocks, Houses, etc.)
    for (int i = 0; i < MAX_DECORATIONS; i++) {
        if (world->state.decos[i].type == DECO_NONE) continue;
        Vector3 pos = {(float)world->state.decos[i].position.x, 0, (float)world->state.decos[i].position.z};
        float dist = Vector3Distance(camera.target, pos);
        if (dist > RENDER_DISTANCE + 20) continue;

        if (drawShadows) DrawDecorationShadow(world->state.decos[i]);

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
            case DECO_SHIP:
                DrawCube((Vector3){pos.x, 0.4f, pos.z}, 6.0f, 0.8f, 2.0f, BROWN); // Hull
                DrawCylinder((Vector3){pos.x, 1.6f, pos.z}, 0.15f, 0.15f, 2.8f, 6, DARKBROWN); // Mast
                DrawCube((Vector3){pos.x + 0.8f, 2.4f, pos.z}, 2.5f, 1.2f, 0.1f, BEIGE); // Sail
                break;
            case DECO_COLUMN:
                DrawCylinder((Vector3){pos.x, 0, pos.z}, 0.25f, 0.25f, 3.0f, 8, LIGHTGRAY);
                DrawCube((Vector3){pos.x, 3.1f, pos.z}, 0.8f, 0.2f, 0.8f, WHITE);
                break;
            case DECO_FORUM_ARCH:
                DrawCube((Vector3){pos.x, 1.0f, pos.z}, 4.0f, 2.0f, 1.0f, LIGHTGRAY);
                DrawCube((Vector3){pos.x, 2.2f, pos.z}, 4.5f, 0.4f, 1.2f, WHITE);
                break;
            case DECO_FIRE_PIT:
            {
                float t = (float)GetTime();
                DrawCylinder((Vector3){pos.x, 0.1f, pos.z}, 0.9f, 0.9f, 0.25f, 8, DARKGRAY);
                // Flickering flame triangles
                float flicker = sinf(t * 8.0f) * 0.08f;
                float height = 0.9f + flicker;
                Vector3 tip = {pos.x, 0.25f + height, pos.z};
                Vector3 left = {pos.x - 0.35f, 0.25f, pos.z};
                Vector3 right = {pos.x + 0.35f, 0.25f, pos.z};
                Vector3 front = {pos.x, 0.25f, pos.z - 0.35f};
                Vector3 back = {pos.x, 0.25f, pos.z + 0.35f};

                DrawTriangle3D(left, tip, right, ORANGE);
                DrawTriangle3D(right, tip, left, ORANGE);
                DrawTriangle3D(front, tip, back, RED);
                DrawTriangle3D(back, tip, front, RED);

                Vector3 tip2 = {pos.x + 0.1f, 0.25f + height * 0.7f, pos.z - 0.05f};
                Vector3 l2 = {pos.x - 0.15f, 0.25f, pos.z - 0.05f};
                Vector3 r2 = {pos.x + 0.25f, 0.25f, pos.z + 0.05f};
                DrawTriangle3D(l2, tip2, r2, YELLOW);
                DrawTriangle3D(r2, tip2, l2, YELLOW);

                // Smoke puffs
                float smokeRise = fmodf(t * 0.6f, 1.0f);
                DrawSphere((Vector3){pos.x + 0.05f, 0.9f + smokeRise * 1.6f, pos.z - 0.05f},
                           0.15f + smokeRise * 0.15f, Fade(GRAY, 0.35f));
                DrawSphere((Vector3){pos.x - 0.1f, 1.1f + smokeRise * 1.4f, pos.z + 0.1f},
                           0.12f + smokeRise * 0.12f, Fade(GRAY, 0.25f));
            }
                break;
            case DECO_FIRE_PIT_UNLIT:
            {
                // Draw three logs angled like a teepee
                // Log 1
                DrawCylinderEx((Vector3){pos.x - 0.3f, 0.1f, pos.z}, (Vector3){pos.x, 0.8f, pos.z}, 0.15f, 0.05f, 8, BROWN);
                // Log 2
                DrawCylinderEx((Vector3){pos.x + 0.3f, 0.1f, pos.z}, (Vector3){pos.x, 0.8f, pos.z}, 0.15f, 0.05f, 8, BROWN);
                // Log 3
                DrawCylinderEx((Vector3){pos.x, 0.1f, pos.z - 0.3f}, (Vector3){pos.x, 0.8f, pos.z}, 0.15f, 0.05f, 8, BROWN);
            }
                break;
            case DECO_SNAKE:
            {
                float t = (float)GetTime();
                float radius = 1.4f;
                Vector3 center = pos;
                Vector3 sPos = {center.x + cosf(t * 1.5f) * radius,
                                0.12f + sinf(t * 2.0f) * 0.02f,
                                center.z + sinf(t * 1.5f) * radius};

                if (snakeModelLoaded) {
                    float heading = -t * 86.0f;
                    Vector3 modelPos = Vector3Add(sPos, snakeModelOffset);
                    DrawModelEx(snakeModel, modelPos, (Vector3){0.0f, 1.0f, 0.0f}, heading, snakeModelScale, WHITE);
                } else {
                    DrawCube((Vector3){sPos.x, 0.1f, sPos.z}, 1.0f, 0.15f, 0.6f, DARKGREEN);
                    DrawSphere((Vector3){sPos.x + 0.45f, 0.2f, sPos.z}, 0.2f, GREEN);
                }
            }
                break;
            default: break;
        }
    }

    // 3.5 Ports
    for (int i = 0; i < world->state.portCount; i++) {
        if (!world->state.ports[i].active) continue;
        Vector3 pos = {(float)world->state.ports[i].position.x, 0, (float)world->state.ports[i].position.z};
        float dist = Vector3Distance(camera.target, pos);
        if (dist > RENDER_DISTANCE + 40) continue;
        DrawCube((Vector3){pos.x, 0.05f, pos.z}, 1.6f, 0.1f, 1.6f, BROWN);
        DrawCube((Vector3){pos.x, 0.6f, pos.z}, 0.2f, 1.0f, 0.2f, DARKBROWN);
        DrawSphere((Vector3){pos.x, 1.2f, pos.z}, 0.15f, GOLD);
    }

    // 4. Ground Items & 5. NPCs
    for (int i = 0; i < MAX_GROUND_ITEMS; i++) {
        if (!world->state.items[i].active) continue;
        if (Vector3Distance(camera.target, (Vector3){(float)world->state.items[i].position.x, 0, (float)world->state.items[i].position.z}) < RENDER_DISTANCE) {
            DrawGroundItemModel(camera,
                                (Vector3){(float)world->state.items[i].position.x, 0, (float)world->state.items[i].position.z},
                                world->state.items[i].itemId);
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
    if (pos.x < world->state.minX || pos.x > world->state.maxX ||
        pos.z < world->state.minZ || pos.z > world->state.maxZ) {
        return true;
    }
    if (IsWaterTile(world, pos)) return true;
    for (int i = 0; i < MAX_DECORATIONS; i++) {
        if (world->state.decos[i].type == DECO_NONE) continue;
        // Simple bounding box check based on decoration type
        int halfSize = 1;
        if (world->state.decos[i].type == DECO_SYNAGOGUE) halfSize = 3;
        else if (world->state.decos[i].type == DECO_TEMPLE) halfSize = 6;
        else if (world->state.decos[i].type == DECO_HOUSE) halfSize = 2;
        else if (world->state.decos[i].type == DECO_SHIP) halfSize = 3;
        else if (world->state.decos[i].type == DECO_FORUM_ARCH) halfSize = 2;
        else if (world->state.decos[i].type == DECO_SNAKE) halfSize = 0;
        
        if (pos.x >= world->state.decos[i].position.x - halfSize && 
            pos.x <= world->state.decos[i].position.x + halfSize &&
            pos.z >= world->state.decos[i].position.z - halfSize &&
            pos.z <= world->state.decos[i].position.z + halfSize) {
            return true;
        }
    }
    return false;
}

bool IsWaterTile(World *world, Vector3Int pos) {
    if (world->state.landPolyCount > 2) {
        int inside = 0;
        for (int i = 0, j = world->state.landPolyCount - 1; i < world->state.landPolyCount; j = i++) {
            Vector2 pi = world->state.landPoly[i];
            Vector2 pj = world->state.landPoly[j];
            bool intersect = ((pi.y > pos.z) != (pj.y > pos.z)) &&
                (pos.x < (pj.x - pi.x) * (pos.z - pi.y) / (pj.y - pi.y + 0.0001f) + pi.x);
            if (intersect) inside = !inside;
        }
        if (inside) return false;
    }
    for (int i = 0; i < world->state.waterCount; i++) {
        WaterRegion r = world->state.water[i];
        if (pos.x >= r.minX && pos.x <= r.maxX && pos.z >= r.minZ && pos.z <= r.maxZ) {
            return true;
        }
    }
    return false;
}

int GetPortAt(World *world, Vector3Int pos) {
    for (int i = 0; i < world->state.portCount; i++) {
        if (!world->state.ports[i].active) continue;
        int dx = abs(pos.x - world->state.ports[i].position.x);
        int dz = abs(pos.z - world->state.ports[i].position.z);
        if (dx <= 1 && dz <= 1) return i;
    }
    return -1;
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

int GetClickedDecoration(World *world, Ray ray) {
    for (int i = 0; i < MAX_DECORATIONS; i++) {
        if (world->state.decos[i].type == DECO_NONE) continue;
        Vector3 pos = {(float)world->state.decos[i].position.x, 0, (float)world->state.decos[i].position.z};
        // Generic box for interaction (mostly for Fire Pit and Snake)
        BoundingBox box = {
            (Vector3){pos.x - 0.6f, 0, pos.z - 0.6f},
            (Vector3){pos.x + 0.6f, 1.0f, pos.z + 0.6f}
        };
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

#define A_STAR_NODES 1024

typedef struct {
    Vector3Int pos;
    int parentIdx;
    float gCost;
    float hCost;
    bool closed;
} AStarNode;

static float GetHeuristic(Vector3Int a, Vector3Int b) {
    // Euclidean distance for 8-way movement
    float dx = (float)(a.x - b.x);
    float dz = (float)(a.z - b.z);
    return sqrtf(dx*dx + dz*dz);
}

void FindPath(World *world, Player *player, Vector3Int finalTarget) {
    if (IsTileBlocked(world, finalTarget)) return;

    static AStarNode nodes[A_STAR_NODES];
    int nodeCount = 0;

    // Start node
    nodes[nodeCount++] = (AStarNode){
        player->position, -1, 0, GetHeuristic(player->position, finalTarget), false
    };

    int foundIdx = -1;
    while (nodeCount < A_STAR_NODES - 9) {
        // Find best open node
        int currentIdx = -1;
        float minFCost = 999999.0f;
        for (int i = 0; i < nodeCount; i++) {
            if (!nodes[i].closed) {
                float fCost = nodes[i].gCost + nodes[i].hCost;
                if (fCost < minFCost) {
                    minFCost = fCost;
                    currentIdx = i;
                }
            }
        }

        if (currentIdx == -1) break; // No path found
        
        AStarNode current = nodes[currentIdx];
        nodes[currentIdx].closed = true;

        if (current.pos.x == finalTarget.x && current.pos.z == finalTarget.z) {
            foundIdx = currentIdx;
            break;
        }

        // Limit search distance
        if (nodeCount > 800) break;
        
        // 8 directions (including diagonals)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dz = -1; dz <= 1; dz++) {
                if (dx == 0 && dz == 0) continue;
                
                Vector3Int nextPos = {current.pos.x + dx, 0, current.pos.z + dz};
                if (!IsTileBlocked(world, nextPos)) {
                    // Cost is 1.0 for orthogonal, ~1.414 for diagonal
                    float moveCost = (dx != 0 && dz != 0) ? 1.414f : 1.0f;
                    float newGCost = current.gCost + moveCost;
                    
                    // Check if already in nodes
                    int existingIdx = -1;
                    for (int i = 0; i < nodeCount; i++) {
                        if (nodes[i].pos.x == nextPos.x && nodes[i].pos.z == nextPos.z) {
                            existingIdx = i;
                            break;
                        }
                    }

                    if (existingIdx == -1) {
                        nodes[nodeCount++] = (AStarNode){
                            nextPos, currentIdx, newGCost, GetHeuristic(nextPos, finalTarget), false
                        };
                    } else if (newGCost < nodes[existingIdx].gCost) {
                        nodes[existingIdx].gCost = newGCost;
                        nodes[existingIdx].parentIdx = currentIdx;
                        nodes[existingIdx].closed = false; // Re-open if we found a better path
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
            tempPath[pathIdx++] = nodes[curr].pos;
            curr = nodes[curr].parentIdx;
        }
        
        // Reverse and set in player
        player->pathSize = 0;
        // Skip current tile (tempPath[pathIdx-1] is the player's current position)
        for (int i = pathIdx - 2; i >= 0; i--) {
            player->path[player->pathSize++] = tempPath[i];
        }
        player->pathIndex = 0;
        player->finalTarget = finalTarget;
        if (player->pathSize > 0) player->target = player->path[0];
    } else {
        // If no path found, just set a straight line target for now
        player->pathSize = 0;
        player->target = finalTarget;
        player->finalTarget = finalTarget;
    }
}
