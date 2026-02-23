#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "world.h"
#include "ui.h"
#include "combat.h"
#include "scripture.h"
#include <stdio.h>

Texture2D spriteDatabase[SPRITE_COUNT];

void LoadSprites() {
    Image imgPaul = GenImageChecked(32, 64, 8, 8, BLUE, WHITE);
    spriteDatabase[SPRITE_PAUL] = LoadTextureFromImage(imgPaul);
    UnloadImage(imgPaul);

    Image imgSadducee = GenImageChecked(32, 64, 8, 8, RED, BLACK);
    spriteDatabase[SPRITE_SADDUCEE] = LoadTextureFromImage(imgSadducee);
    UnloadImage(imgSadducee);

    Image imgAnanias = GenImageChecked(32, 64, 8, 8, GREEN, WHITE);
    spriteDatabase[SPRITE_ANANIAS] = LoadTextureFromImage(imgAnanias);
    UnloadImage(imgAnanias);
}

int main() {
    // 1. Initialization
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "RayScape - Paul's Journeys");
    LoadSprites();

    // Fixed isometric-style Camera
    Camera3D camera = {0};
    camera.position = (Vector3){8.0f, 8.0f, 8.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize State
    Player player = {0};
    InitPlayer(&player);

    World world = {0};
    InitWorld(&world);

    CombatSession combat = {0};

    SetTargetFPS(60);

    // --- Main Loop ---
    while (!WindowShouldClose()) {
        // 2. Logic: Mouse Interaction
        if (!combat.active && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), camera);
            
            // Check NPC first
            int npcIdx = GetClickedNPC(&world, ray);
            if (npcIdx != -1) {
                const char *npcName = world.state.npcs[npcIdx].name;
                
                if (TextIsEqual(npcName, "Sadducee")) {
                    if (player.questStates[1] == QUEST_NOT_STARTED) {
                        player.activeQuestId = 1;
                        player.questStates[1] = QUEST_ACTIVE;
                    } else if (player.questStates[1] == QUEST_ACTIVE && player.position.x == 150 && player.position.z == 500) {
                        player.questStates[1] = QUEST_COMPLETED;
                        player.activeQuestId = 0;
                        player.skills[SKILL_ORATORY].currentXp += 500;
                    } else {
                        StartCombat(&combat, npcName);
                    }
                } else if (TextIsEqual(npcName, "Ananias")) {
                    if (player.questStates[2] == QUEST_NOT_STARTED) {
                        player.activeQuestId = 2;
                        player.questStates[2] = QUEST_ACTIVE;
                    }
                } else {
                    StartCombat(&combat, npcName);
                }
            } else {
                // Check Ground Item
                int itemIdx = GetClickedItem(&world, ray);
                if (itemIdx != -1) {
                    if (AddToInventory(&player, world.state.items[itemIdx].itemId)) {
                        world.state.items[itemIdx].active = false;
                    }
                } else {
                    // Otherwise move player to grid clicked
                    Vector3Int gridClick = GetGridClicked(ray);
                    if (gridClick.y != -1 && !IsTileBlocked(&world, gridClick)) {
                        player.target = gridClick;
                    }
                }
            }
        }

        if (combat.active) {
            UpdateCombat(&combat, &player);
        } else {
            UpdatePlayer(&player);
            UpdateWorld(&world, &player);
        }

        // --- FIXED FOLLOW CAMERA ---
        // Offset is (8, 8, 8) relative to player position
        camera.target = player.lerpPosition;
        camera.position = (Vector3){player.lerpPosition.x + 8.0f, 8.0f, player.lerpPosition.z + 8.0f};

        // 3. Rendering
        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        DrawWorld(&world, camera);
        DrawPlayer(&player, camera);

        // Destination Marker
        if (!combat.active && Vector3Distance(player.lerpPosition, (Vector3){(float)player.target.x, (float)player.target.y, (float)player.target.z}) > 0.1f) {
            DrawCircle3D((Vector3){(float)player.target.x, 0.01f, (float)player.target.z}, 0.5f, (Vector3){1, 0, 0}, 90.0f, Fade(YELLOW, 0.5f));
        }
        EndMode3D();

        // UI Layer
        DrawInventory(&player, screenWidth, screenHeight);
        DrawSkills(&player);
        DrawHUD(&player, &world);

        if (combat.active) DrawCombatUI(&combat, screenWidth, screenHeight);

        DrawFPS(10, screenHeight - 25);
        DrawText("Point & Click to move across the Mediterranean", 10, screenHeight - 45, 15, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
