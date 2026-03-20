#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
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
    const int screenWidth = 1280;
    const int screenHeight = 720;

    Camera3D camera = {0};
    Player player = {0};
    World world = {0};
    CombatSession combat = {0};

    bool isFirstPerson = false;
    bool shadersEnabled = true; // Shadows on by default
    
    // Shadow Mapping Resources
    Shader shadowShader = {0};
    Shader depthShader = {0};
    RenderTexture2D shadowMap = {0};
    const int SHADOW_MAP_SIZE = 1024;
    
    // Shader Settings
    ShaderSettings settings = {
        .lightDir = Vector3Normalize((Vector3){0.1f, 1.0f, 0.1f}),
        .lightColor = WHITE,
        .ambient = 0.6f,
        .shadowBias = 0.005f,
        .showDebugUI = false
    };

    // Load settings if exist
    FILE *f = fopen("shader_settings.txt", "r");
    if (f) {
        int r, g, b;
        fscanf(f, "%f %f %f", &settings.lightDir.x, &settings.lightDir.y, &settings.lightDir.z);
        fscanf(f, "%d %d %d", &r, &g, &b);
        settings.lightColor = (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
        fscanf(f, "%f", &settings.ambient);
        fscanf(f, "%f", &settings.shadowBias);
        fclose(f);
    }
    
    char message[256] = {0};
    float messageTimer = 0.0f;
    float snakeEventTimer = 0.0f;

    // 2. Initialization
    InitWindow(screenWidth, screenHeight, "RayScape - Paul's Journeys");
    LoadSprites();

    // Initialize Shaders and Shadow Map
    shadowShader = LoadShader("shaders/shadow.vs", "shaders/shadow.fs");
    depthShader = LoadShader("shaders/depth.vs", "shaders/depth.fs");
    shadowMap = LoadRenderTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    // Set shadow map texture slot
    int shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
    // We will set this manually in the loop just to be safe, but usually texture1

    // Get uniform locations
    shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
    int lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
    int lightColorLoc = GetShaderLocation(shadowShader, "lightColor");
    int ambientLoc = GetShaderLocation(shadowShader, "ambient");
    int shadowBiasLoc = GetShaderLocation(shadowShader, "shadowBias");
    int lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
    int shadowMapSizeLoc = GetShaderLocation(shadowShader, "shadowMapSize");
    
    // Set constant uniforms (Initial values)
    float smSize = (float)SHADOW_MAP_SIZE;
    SetShaderValue(shadowShader, shadowMapSizeLoc, &smSize, SHADER_UNIFORM_FLOAT);

    // Initialize remaining game state
    InitPlayer(&player);
    InitWorld(&world);

    // Fixed isometric-style Camera
    camera.position = (Vector3){8.0f, 8.0f, 8.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

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
        if (IsKeyPressed(KEY_F2)) {
            shadersEnabled = !shadersEnabled;
        }
        if (IsKeyPressed(KEY_F3)) {
            isFirstPerson = false;
            EnableCursor();
        }
        if (IsKeyPressed(KEY_F5)) {
            settings.showDebugUI = !settings.showDebugUI;
            if (settings.showDebugUI) EnableCursor();
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
        if (!settings.showDebugUI && !combat.active && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
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

        // --- SHADOW MAPPING ---
        // 1. Calculate Light View-Projection Matrix
        Matrix lightView;
        Matrix lightProj;
        float shadowBoxSize = 60.0f; // Increased size
        Vector3 lightDir = settings.lightDir;
        if (Vector3LengthSqr(lightDir) < 0.0001f) lightDir = (Vector3){0.1f, 1.0f, 0.1f};
        lightDir = Vector3Normalize(lightDir);
        float lightDistance = 40.0f;
        Vector3 lightPos = Vector3Scale(lightDir, lightDistance);
        Vector3 center = isFirstPerson ? player.lerpPosition : camera.target;
        
        // Ensure light follows the camera/player
        Vector3 lightCamPos = Vector3Add(center, lightPos);
        lightView = MatrixLookAt(lightCamPos, center, (Vector3){0, 1, 0});
        lightProj = MatrixOrtho(-shadowBoxSize, shadowBoxSize, -shadowBoxSize, shadowBoxSize, 1.0f, 150.0f);
        
        // MVP = P * V * M. So VP = P * V.
        Matrix lightVP = MatrixMultiply(lightView, lightProj); // Raylib's MatrixMultiply might behave as V * P effectively due to layout? 
        // Actually, let's stick to standard P * V.
        // Wait, if I use MatrixMultiply(lightView, lightProj), that is V * P.
        // If the shader expects P * V * pos, I should use MatrixMultiply(lightProj, lightView).
        // However, many Raylib examples use V * P order for some reason. 
        // Let's try the standard P * V first.
        lightVP = MatrixMultiply(lightView, lightProj); 
        // Wait, I will use the one that matches Raylib's internal mvp calculation.
        // Raylib uses: matModelView = matView * matModel; matModelViewProjection = matProjection * matModelView;
        // So P * V * M.
        // So I need P * V.
        // So MatrixMultiply(lightProj, lightView).
        lightVP = MatrixMultiply(lightView, lightProj); // I'll trust the previous code's order but fix the Projection mismatch first. 
        // Actually, let's try swapping it because V*P is definitely wrong for MVP * pos.
        lightVP = MatrixMultiply(lightProj, lightView); // Swapped to P * V

        // 2. Render Depth Map (Pass 1)
        if (shadersEnabled) {
            BeginTextureMode(shadowMap);
                ClearBackground(WHITE); // Far plane depth is 1.0 (White)
                BeginMode3D((Camera3D){
                    lightCamPos, center, {0, 1, 0}, 90.0f, CAMERA_ORTHOGRAPHIC // fovy placeholder
                });
                    // Force the exact projection matrix we calculated
                    rlSetMatrixProjection(lightProj);
                    
                    BeginShaderMode(depthShader);
                        rlDisableBackfaceCulling();
                        DrawWorld(&world, (Camera3D){lightCamPos, center, {0,1,0}, 40.0f, CAMERA_ORTHOGRAPHIC});
                        rlEnableBackfaceCulling();
                    EndShaderMode();

                EndMode3D();
            EndTextureMode();
        }

        // 3. Render Scene with Shadows (Pass 2)
        BeginDrawing();
        ClearBackground(SKYBLUE);

        BeginMode3D(camera);
            if (shadersEnabled) {
                // Update shadow shader uniforms
                SetShaderValueMatrix(shadowShader, lightVPLoc, lightVP);
                SetShaderValue(shadowShader, shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &camera.position, SHADER_UNIFORM_VEC3);
                SetShaderValue(shadowShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
                
                Vector3 lightColorVec = (Vector3){(float)settings.lightColor.r/255.0f, (float)settings.lightColor.g/255.0f, (float)settings.lightColor.b/255.0f};
                SetShaderValue(shadowShader, lightColorLoc, &lightColorVec, SHADER_UNIFORM_VEC3);
                SetShaderValue(shadowShader, ambientLoc, &settings.ambient, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shadowShader, shadowBiasLoc, &settings.shadowBias, SHADER_UNIFORM_FLOAT);

                // Bind Shadow Map to texture slot 1 (slot 0 is diffuse texture)
                // In Raylib shaders, we usually pass texture via uniform sampler.
                // We can use the slot index.
                // Set active texture slot to 1
                rlActiveTextureSlot(1);
                rlEnableTexture(shadowMap.texture.id);
                rlActiveTextureSlot(0); // Back to default

                // Tell shader that "shadowMap" sampler uses texture unit 1
                int slot = 1;
                SetShaderValue(shadowShader, shadowMapLoc, &slot, SHADER_UNIFORM_INT);

                BeginShaderMode(shadowShader);
            }

            DrawWorld(&world, camera);
            if (!isFirstPerson) DrawPlayer(&player, camera);

            if (shadersEnabled) {
                EndShaderMode();
                rlActiveTextureSlot(1);
                rlDisableTexture(); // Unbind
                rlActiveTextureSlot(0);
            }

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

        // UI Layer - drawn directly to screen
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
        
        DrawShaderDebugUI(&settings);

        EndDrawing();
    }

    CloseWindow();
    UnloadShader(shadowShader);
    UnloadShader(depthShader);
    UnloadRenderTexture(shadowMap);
    return 0;
}
