#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "player.h"
#include "world.h"
#include "ui.h"
#include "combat.h"
#include "scripture.h"
#include <stdio.h>
#include <stdlib.h> // <-- system() is here
#include <math.h>

Texture2D spriteDatabase[SPRITE_COUNT];
Model snakeModel = {0};
bool snakeModelLoaded = false;
Vector3 snakeModelScale = {1.0f, 1.0f, 1.0f};
Vector3 snakeModelOffset = {0};
Model columnModel = {0};
bool columnModelLoaded = false;
Vector3 columnModelScale = {1.0f, 1.0f, 1.0f};
Vector3 columnModelOffset = {0};
Model templeModel = {0};
bool templeModelLoaded = false;
Vector3 templeModelScale = {1.0f, 1.0f, 1.0f};
Vector3 templeModelOffset = {0};
Model desertHouseModel = {0};
bool desertHouseModelLoaded = false;
Vector3 desertHouseModelScale = {1.0f, 1.0f, 1.0f};
Vector3 desertHouseModelOffset = {0};
Model sadduceeModel = {0};
bool sadduceeModelLoaded = false;
Vector3 sadduceeModelScale = {1.0f, 1.0f, 1.0f};
Vector3 sadduceeModelOffset = {0};

// ---------- MODEL LOADING ----------
static void LoadModels(void) {
    if (FileExists("assets/snake.glb")) {
        snakeModel = LoadModel("assets/snake.glb");
        snakeModelLoaded = snakeModel.meshCount > 0;
    }

    if (snakeModelLoaded) {
        for (int i = 0; i < snakeModel.materialCount; i++) {
            snakeModel.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = (Color){82, 128, 64, 255};
        }

        BoundingBox bounds = GetModelBoundingBox(snakeModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxDim = fmaxf(sizeX, fmaxf(sizeY, sizeZ));
        if (maxDim < 0.001f) maxDim = 1.0f;

        float scale = 1.2f / maxDim;
        snakeModelScale = (Vector3){scale, scale, scale};
        snakeModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale) + 0.02f,
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/column.glb")) {
        columnModel = LoadModel("assets/column.glb");
        columnModelLoaded = columnModel.meshCount > 0;
    }

    if (columnModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(columnModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxXZ = fmaxf(sizeX, sizeZ);
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (maxXZ < 0.001f) maxXZ = 1.0f;

        float scaleY = 3.0f / sizeY;
        float scaleXZ = 0.6f / maxXZ;
        float scale = fminf(scaleY, scaleXZ);
        columnModelScale = (Vector3){scale, scale, scale};
        columnModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/roman_temple.glb")) {
        templeModel = LoadModel("assets/roman_temple.glb");
        templeModelLoaded = templeModel.meshCount > 0;
    }

    if (templeModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(templeModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        if (sizeX < 0.001f) sizeX = 1.0f;
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (sizeZ < 0.001f) sizeZ = 1.0f;

        float scaleX = 10.5f / sizeX;
        float scaleY = 6.0f / sizeY;
        float scaleZ = 12.5f / sizeZ;
        float scale = fminf(scaleX, fminf(scaleY, scaleZ));
        templeModelScale = (Vector3){scale, scale, scale};
        templeModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/desert_house.glb")) {
        desertHouseModel = LoadModel("assets/desert_house.glb");
        desertHouseModelLoaded = desertHouseModel.meshCount > 0;
    }

    if (desertHouseModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(desertHouseModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        if (sizeX < 0.001f) sizeX = 1.0f;
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (sizeZ < 0.001f) sizeZ = 1.0f;

        float scaleX = 16.8f / sizeX;
        float scaleY = 13.6f / sizeY;
        float scaleZ = 16.8f / sizeZ;
        float scale = fminf(scaleX, fminf(scaleY, scaleZ));
        desertHouseModelScale = (Vector3){scale, scale, scale};
        desertHouseModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/ancient_character.glb")) {
        sadduceeModel = LoadModel("assets/ancient_character.glb");
        sadduceeModelLoaded = sadduceeModel.meshCount > 0;
    }

    if (sadduceeModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(sadduceeModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxXZ = fmaxf(sizeX, sizeZ);
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (maxXZ < 0.001f) maxXZ = 1.0f;

        float scaleY = 1.8f / sizeY;
        float scaleXZ = 1.2f / maxXZ;
        float scale = fminf(scaleY, scaleXZ);
        sadduceeModelScale = (Vector3){scale, scale, scale};
        sadduceeModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }
}

// ---------- UNLOAD MODELS ----------
static void UnloadModels(void) {
    if (snakeModelLoaded) { UnloadModel(snakeModel); snakeModelLoaded = false; }
    if (columnModelLoaded) { UnloadModel(columnModel); columnModelLoaded = false; }
    if (templeModelLoaded) { UnloadModel(templeModel); templeModelLoaded = false; }
    if (desertHouseModelLoaded) { UnloadModel(desertHouseModel); desertHouseModelLoaded = false; }
    if (sadduceeModelLoaded) { UnloadModel(sadduceeModel); sadduceeModelLoaded = false; }
}

// ---------- PLAYER PORT MOVEMENT ----------
static void MovePlayerToPort(Player *player, World *world, int portIdx) {
    Vector3Int p = world->state.ports[portIdx].position;
    player->position = p;
    player->target = p;
    player->finalTarget = p;
    player->pathSize = 0;
    player->pathIndex = 0;
    player->lerpPosition = (Vector3){(float)p.x, 0.0f, (float)p.z};
}

// ---------- PORT TRAVEL ----------
static void HandlePortTravel(Player *player, World *world, int portIdx) {
    if (world->state.nextWorldId == WORLD_NONE) return;

    // -----------------------------
    // RUN MINI-LEVEL HERE
    // -----------------------------
    // Replace "mini_level.exe" with your executable
    int ret = system("mini_level.exe"); 
    if (ret != 0) {
        printf("Warning: failed to launch mini-level! Return code: %d\n", ret);
    } else {
        printf("Mini-level finished, returning to main game.\n");
    }

    // Continue normal world travel
    LoadWorld(world, world->state.nextWorldId);
    printf("Moved worlds!\n");    

    if (world->state.portCount > 0) {
        MovePlayerToPort(player, world, 0);
    }
}

// ---------- SPRITES ----------
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

    Image imgLetter = GenImageColor(48, 48, BLANK);
    ImageDrawRectangle(&imgLetter, 8, 12, 32, 22, (Color){244, 232, 196, 255});
    ImageDrawRectangleLines(&imgLetter, (Rectangle){8, 12, 32, 22}, 2, DARKBROWN);
    ImageDrawLine(&imgLetter, 8, 12, 24, 24, DARKBROWN);
    ImageDrawLine(&imgLetter, 40, 12, 24, 24, DARKBROWN);
    ImageDrawCircleV(&imgLetter, (Vector2){34, 28}, 3, RED);
    spriteDatabase[SPRITE_LETTER] = LoadTextureFromImage(imgLetter);
    UnloadImage(imgLetter);
}

// ---------- MAIN LOOP ----------
int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    Camera3D camera = {0};
    Player player = {0};
    World world = {0};
    CombatSession combat = {0};

    bool isFirstPerson = false;
    bool shadersEnabled = false;

    Shader shadowShader = {0};
    Shader depthShader = {0};
    RenderTexture2D shadowMap = {0};
    const int SHADOW_MAP_SIZE = 1024;

    ShaderSettings settings = {
        .lightDir = (Vector3){0.0f, 1.0f, 0.0f},
        .lightColor = WHITE,
        .ambient = 0.6f,
        .shadowBias = 0.005f,
        .showDebugUI = false
    };

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

    InitWindow(screenWidth, screenHeight, "RayScape - Paul's Journeys");
    LoadSprites();
    LoadModels();

    shadowShader = LoadShader("shaders/shadow.vs", "shaders/shadow.fs");
    depthShader = LoadShader("shaders/depth.vs", "shaders/depth.fs");
    shadowMap = LoadRenderTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    InitPlayer(&player);
    InitWorld(&world);

    camera.position = (Vector3){8.0f, 8.0f, 8.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    SetTargetFPS(60);

    // --------------- MAIN LOOP ---------------
    while (!WindowShouldClose()) {
        if (messageTimer > 0) messageTimer -= GetFrameTime();
        if (snakeEventTimer > 0) {
            snakeEventTimer -= GetFrameTime();
            if (snakeEventTimer <= 0 && player.questStates[3] == QUEST_ACTIVE) {
                player.questStates[3] = QUEST_COMPLETED;
                for(int i=0;i<MAX_DECORATIONS;i++)
                    if(world.state.decos[i].type==DECO_SNAKE)
                        world.state.decos[i].type=DECO_NONE;
                snprintf(message,sizeof(message),"You shake off the creature into the fire and suffer no harm.");
                messageTimer=5.0f;
            }
        }

        // Perspective Switching
        if (IsKeyPressed(KEY_F1)) { isFirstPerson=true; DisableCursor(); }
        if (IsKeyPressed(KEY_F2)) { shadersEnabled=!shadersEnabled; }
        if (IsKeyPressed(KEY_F3)) { isFirstPerson=false; EnableCursor(); }
        if (IsKeyPressed(KEY_F5)) { settings.showDebugUI=!settings.showDebugUI; if(settings.showDebugUI) EnableCursor(); }

        // Camera
        if (isFirstPerson) {
            Vector2 delta=GetMouseDelta();
            player.yaw -= delta.x*0.005f;
            player.pitch += delta.y*-0.005f;
            if(player.pitch>PI/2.5f) player.pitch=PI/2.5f;
            if(player.pitch<-PI/2.5f) player.pitch=-PI/2.5f;
            camera.position=(Vector3){player.lerpPosition.x,1.6f,player.lerpPosition.z};
            Vector3 look={cosf(player.pitch)*sinf(player.yaw), sinf(player.pitch), cosf(player.pitch)*cosf(player.yaw)};
            camera.target = Vector3Add(camera.position, look);
            camera.fovy=60.0f;
        } else {
            camera.target=player.lerpPosition;
            camera.position=(Vector3){player.lerpPosition.x+8.0f, 8.0f, player.lerpPosition.z+8.0f};
            camera.fovy=45.0f;
        }

        // Mouse Interaction & Movement (simplified)
        if(!settings.showDebugUI && !combat.active && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = isFirstPerson ? GetMouseRay((Vector2){GetScreenWidth()/2,GetScreenHeight()/2},camera) : GetMouseRay(GetMousePosition(),camera);
            int npcIdx = GetClickedNPC(&world,ray);
            if(npcIdx!=-1) { StartCombat(&combat,world.state.npcs[npcIdx].name); }
            else {
                Vector3Int gridClick = GetGridClicked(ray);
                if(gridClick.y!=-1) FindPath(&world,&player,gridClick);
            }
        }

        if(combat.active) UpdateCombat(&combat,&player);
        else {
            int portIdx = GetPortAt(&world, player.position);
            if(portIdx!=-1 && IsKeyPressed(KEY_T)) {
                if(world.state.worldId==WORLD_MALTA && player.questStates[3]!=QUEST_COMPLETED) {
                    snprintf(message,sizeof(message),"Captain: 'We must wait for the winter storms to pass.'");
                    messageTimer=4.0f;
                } else {
                    HandlePortTravel(&player,&world,portIdx);
                }
            }

            if(!player.gameComplete) UpdatePlayer(&player,&world,isFirstPerson);
            UpdateWorld(&world,&player);
            if(IsKeyPressed(KEY_C)) TryCraftTent(&player);
            if(IsKeyPressed(KEY_I)) player.showInventory=!player.showInventory;
            if(IsKeyPressed(KEY_M)) player.showMap=!player.showMap;
        }

        // Drawing (simplified for brevity)
        BeginDrawing();
        ClearBackground(SKYBLUE);
        BeginMode3D(camera);
            DrawWorld(&world,camera,shadersEnabled);
            if(!isFirstPerson) DrawPlayer(&player,camera,shadersEnabled);
        EndMode3D();
        DrawInventory(&player);
        DrawUIMessage(message,messageTimer);
        EndDrawing();
    }

    // Clean-up
    UnloadModels();
    for(int i=0;i<SPRITE_COUNT;i++) UnloadTexture(spriteDatabase[i]);
    CloseWindow();
    return 0;
}