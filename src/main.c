#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "world.h"
#include "ui.h"
#include "combat.h"
#include "scripture.h"
#include <stdio.h>
#include <stdlib.h>

Texture2D spriteDatabase[SPRITE_COUNT];

static void MovePlayerToPort(Player *player, World *world, int portIdx) {
    Vector3Int p = world->state.ports[portIdx].position;
    player->position = p;
    player->target = p;
    player->finalTarget = p;
    player->pathSize = 0;
    player->pathIndex = 0;
    player->lerpPosition = (Vector3){(float)p.x, 0.0f, (float)p.z};
}

static void HandlePortTravel(Player *player, World *world, int portIdx) {
    if (world->state.nextWorldId == WORLD_NONE) return;
    LoadWorld(world, world->state.nextWorldId);
    if (world->state.portCount > 0) {
        MovePlayerToPort(player, world, 0);
    }
}

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

    bool isFirstPerson = false;
    
    char message[256] = {0};
    float messageTimer = 0.0f;
    float snakeEventTimer = 0.0f;

    SetTargetFPS(60);

    // --- Main Loop ---
    while (!WindowShouldClose()) {
        if (messageTimer > 0) messageTimer -= GetFrameTime();
        
        if (snakeEventTimer > 0) {
            snakeEventTimer -= GetFrameTime();
            if (snakeEventTimer <= 0) {
                if (player.questStates[3] == QUEST_ACTIVE) {
                    player.questStates[3] = QUEST_COMPLETED;
                    // Remove snake
                    for(int i=0; i<MAX_DECORATIONS; i++) {
                         if(world.state.decos[i].type == DECO_SNAKE) {
                             world.state.decos[i].type = DECO_NONE;
                         }
                    }
                    snprintf(message, sizeof(message), "You shake off the creature into the fire and suffer no harm.");
                    messageTimer = 5.0f;
                }
            }
        }

        // Handle perspective switching
        if (IsKeyPressed(KEY_F1)) {
            isFirstPerson = true;
            DisableCursor();
        }
        if (IsKeyPressed(KEY_F3)) {
            isFirstPerson = false;
            EnableCursor();
        }

        // --- CAMERA UPDATE ---
        if (isFirstPerson) {
            // Mouse rotation
            Vector2 delta = GetMouseDelta();
            player.yaw -= delta.x * 0.005f;
            player.pitch += delta.y * -0.005f;
            if (player.pitch > PI/2.5f) player.pitch = PI/2.5f;
            if (player.pitch < -PI/2.5f) player.pitch = -PI/2.5f;

            // Camera at player position, looking towards yaw/pitch
            camera.position = (Vector3){player.lerpPosition.x, 1.6f, player.lerpPosition.z};
            
            Vector3 look = {
                cosf(player.pitch) * sinf(player.yaw),
                sinf(player.pitch),
                cosf(player.pitch) * cosf(player.yaw)
            };
            camera.target = Vector3Add(camera.position, look);
            camera.fovy = 60.0f;
        } else {
            // 3rd Person (Runescape Style)
            camera.target = player.lerpPosition;
            camera.position = (Vector3){player.lerpPosition.x + 8.0f, 8.0f, player.lerpPosition.z + 8.0f};
            camera.fovy = 45.0f;
        }

        // 2. Logic: Interaction
        if (!combat.active && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray;
            if (isFirstPerson) {
                ray = GetMouseRay((Vector2){(float)GetScreenWidth()/2, (float)GetScreenHeight()/2}, camera);
            } else {
                ray = GetMouseRay(GetMousePosition(), camera);
            }
            
            // Check NPC first
            int npcIdx = GetClickedNPC(&world, ray);
            if (npcIdx != -1) {
                const char *npcName = world.state.npcs[npcIdx].name;
                
                if (TextIsEqual(npcName, "Sadducee")) {
                    if (player.questStates[1] == QUEST_NOT_STARTED) {
                        player.activeQuestId = 1;
                        player.questStates[1] = QUEST_ACTIVE;
                    } else if (player.questStates[1] == QUEST_ACTIVE && player.position.x == 75 && player.position.z == 250) {
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
                } else if (TextIsEqual(npcName, "Islander")) {
                    if (player.questStates[3] == QUEST_COMPLETED) {
                        snprintf(message, sizeof(message), "Islander: 'He must be a god! He suffered no harm!'");
                    } else {
                         snprintf(message, sizeof(message), "Islander: 'No doubt this man is a murderer, for justice has not allowed him to live.'");
                    }
                    messageTimer = 5.0f;
                } else {
                    StartCombat(&combat, npcName);
                }
            } else {
                // Check Ground Item
                int itemIdx = GetClickedItem(&world, ray);
                int decoIdx = GetClickedDecoration(&world, ray);
                
                if (itemIdx != -1) {
                    if (AddToInventory(&player, world.state.items[itemIdx].itemId)) {
                        world.state.items[itemIdx].active = false;
                    }
                } else if (decoIdx != -1) {
                    if (world.state.decos[decoIdx].type == DECO_FIRE_PIT_UNLIT) { // Changed from DECO_FIRE_PIT
                        if (player.questStates[3] == QUEST_NOT_STARTED) {
                            player.questStates[3] = QUEST_ACTIVE;
                            // Change fire pit to lit state
                            world.state.decos[decoIdx].type = DECO_FIRE_PIT;
                            // Reveal snake
                            for(int i=0; i<MAX_DECORATIONS; i++) {
                                if(world.state.decos[i].position.x == world.state.snakePos.x && 
                                   world.state.decos[i].position.z == world.state.snakePos.z &&
                                   world.state.decos[i].type == DECO_NONE) {
                                    world.state.decos[i].type = DECO_SNAKE;
                                    break;
                                }
                            }
                            snprintf(message, sizeof(message), "A viper fastens on your hand! The islanders watch closely...");
                            messageTimer = 5.0f;
                            snakeEventTimer = 8.0f;
                        } else {
                             snprintf(message, sizeof(message), "The fire burns warmly.");
                             messageTimer = 3.0f;
                        }
                    }
                } else {
                    // Otherwise move player to grid clicked
                    if (!isFirstPerson) {
                        Vector3Int gridClick = GetGridClicked(ray);
                        if (gridClick.y != -1) {
                            FindPath(&world, &player, gridClick);
                        }
                    }
                }
            }
        }

        if (combat.active) {
            UpdateCombat(&combat, &player);
        } else {
            int portIdx = GetPortAt(&world, player.position);
            if (portIdx != -1 && IsKeyPressed(KEY_T)) {
                if (world.state.worldId == WORLD_MALTA && player.questStates[3] != QUEST_COMPLETED) {
                    snprintf(message, sizeof(message), "Captain: 'We must wait for the winter storms to pass.'");
                    messageTimer = 4.0f;
                } else {
                    HandlePortTravel(&player, &world, portIdx);
                }
            }

            if (!player.gameComplete) {
                UpdatePlayer(&player, &world, isFirstPerson);
            }
            UpdateWorld(&world, &player);
            
            // Key Bindings
            if (IsKeyPressed(KEY_C)) TryCraftTent(&player);
            if (IsKeyPressed(KEY_I)) player.showInventory = !player.showInventory;
            if (IsKeyPressed(KEY_M)) player.showMap = !player.showMap;

            if (!player.gameComplete && world.state.hasHouseArrest) {
                Vector3Int h = world.state.houseArrestPos;
                if (abs(player.position.x - h.x) <= 3 && abs(player.position.z - h.z) <= 3) {
                    if (IsKeyPressed(KEY_H)) {
                        player.gameComplete = true;
                    }
                }
            }
        }

        // 3. Rendering
        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
        DrawWorld(&world, camera);
        if (!isFirstPerson) DrawPlayer(&player, camera);

        // Objective beacon at next port
        if (world.state.portCount > 0 && world.state.nextWorldId != WORLD_NONE) {
            Vector3Int p = world.state.ports[0].position;
            Vector3 base = {(float)p.x, 0.0f, (float)p.z};
            DrawCylinder(base, 0.35f, 0.35f, 2.0f, 8, Fade(YELLOW, 0.7f));
            DrawSphere((Vector3){base.x, 2.2f, base.z}, 0.2f, YELLOW);
        }

        // Destination Marker
        if (!combat.active && Vector3Distance(player.lerpPosition, (Vector3){(float)player.target.x, (float)player.target.y, (float)player.target.z}) > 0.1f) {
            DrawCircle3D((Vector3){(float)player.target.x, 0.01f, (float)player.target.z}, 0.5f, (Vector3){1, 0, 0}, 90.0f, Fade(YELLOW, 0.5f));
        }
        EndMode3D();

        // UI Layer
        DrawInventory(&player);
        DrawSkills(&player);
        DrawHUD(&player, &world, isFirstPerson);

        if (combat.active) DrawCombatUI(&combat, GetScreenWidth(), GetScreenHeight());
        
        if (messageTimer > 0) {
            DrawRectangle(0, GetScreenHeight() - 100, GetScreenWidth(), 40, Fade(BLACK, 0.7f));
            DrawText(message, 20, GetScreenHeight() - 90, 20, WHITE);
        }

        DrawText("Point & Click to move across the Mediterranean", 10, GetScreenHeight() - 45, 15, WHITE);
        DrawFPS(10, GetScreenHeight() - 25);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
