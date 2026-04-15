#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "player.h"
#include "world.h"
#include "ui.h"
#include "combat.h"
#include "ship_minigame.h"
#include "scripture.h"
#include "platform_input.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if SPPS_PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#define GAME_SCREEN_WIDTH 1280
#define GAME_SCREEN_HEIGHT 720
#define SHADOW_MAP_SIZE 1024

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
Model boatModel = {0};
bool boatModelLoaded = false;
Vector3 boatModelScale = {1.0f, 1.0f, 1.0f};
Vector3 boatModelOffset = {0};
Model islanderModel = {0};
bool islanderModelLoaded = false;
Vector3 islanderModelScale = {1.0f, 1.0f, 1.0f};
Vector3 islanderModelOffset = {0};
Model paulModel = {0};
bool paulModelLoaded = false;
Vector3 paulModelScale = {1.0f, 1.0f, 1.0f};
Vector3 paulModelOffset = {0};
Model romanCharacterModel = {0};
bool romanCharacterModelLoaded = false;
Vector3 romanCharacterModelScale = {1.0f, 1.0f, 1.0f};
Vector3 romanCharacterModelOffset = {0};
Model romanSoldierModel = {0};
bool romanSoldierModelLoaded = false;
Vector3 romanSoldierModelScale = {1.0f, 1.0f, 1.0f};
Vector3 romanSoldierModelOffset = {0};

typedef struct {
    Camera3D camera;
    Player player;
    World world;
    CombatSession combat;
    ShipMinigame shipMinigame;
    bool isFirstPerson;
    bool shadersEnabled;
    bool shadowPipelineReady;
    Shader shadowShader;
    Shader depthShader;
    RenderTexture2D shadowMap;
    int shadowMapLoc;
    int lightDirLoc;
    int lightColorLoc;
    int ambientLoc;
    int shadowBiasLoc;
    int lightVPLoc;
    ShaderSettings settings;
    float snakeEventTimer;
} GameRuntime;

static GameRuntime g_game = {0};

static void DrawStartupLoadingScreen(int screenWidth, int screenHeight, float progress, const char *label) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    Color skyTop = (Color){22, 40, 68, 255};
    Color skyBottom = (Color){173, 126, 71, 255};
    int barWidth = 460;
    int barHeight = 20;
    int barX = screenWidth / 2 - barWidth / 2;
    int barY = screenHeight / 2 + 36;
    int fillWidth = (int)((float)(barWidth - 8) * progress);
    const char *title = "St. Paul's Postal Service";
    const char *subtitle = "Preparing Paul's journey across the Mediterranean";

    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight, skyTop, skyBottom);
    DrawCircle(screenWidth - 170, 120, 58, Fade(GOLD, 0.88f));
    DrawRectangle(0, screenHeight - 180, screenWidth, 180, (Color){18, 55, 96, 255});
    DrawTriangle((Vector2){140, screenHeight - 110}, (Vector2){270, screenHeight - 150}, (Vector2){328, screenHeight - 92}, (Color){92, 60, 29, 255});
    DrawTriangle((Vector2){245, screenHeight - 218}, (Vector2){245, screenHeight - 90}, (Vector2){352, screenHeight - 130}, Fade(RAYWHITE, 0.82f));

    DrawText(title, screenWidth / 2 - MeasureText(title, 54) / 2, screenHeight / 2 - 110, 54, RAYWHITE);
    DrawText(subtitle, screenWidth / 2 - MeasureText(subtitle, 24) / 2, screenHeight / 2 - 56, 24, Fade(RAYWHITE, 0.9f));
    DrawText(label, screenWidth / 2 - MeasureText(label, 24) / 2, screenHeight / 2 + 2, 24, WHITE);

    DrawRectangle(barX, barY, barWidth, barHeight, Fade(BLACK, 0.45f));
    DrawRectangle(barX + 4, barY + 4, fillWidth, barHeight - 8, GOLD);
    DrawRectangleLines(barX, barY, barWidth, barHeight, Fade(RAYWHITE, 0.55f));
    EndDrawing();
}

static void ShowWorldMessage(Player *player, const char *text, float duration) {
    snprintf(player->worldMessage, sizeof(player->worldMessage), "%s", text);
    player->worldMessageTimer = duration;
}

static bool HasDecorationType(const World *world, DecorationType type) {
    for (int i = 0; i < MAX_DECORATIONS; i++) {
        if (world->state.decos[i].type == type) return true;
    }
    return false;
}

static bool IsMaltaFireLit(const World *world) {
    return HasDecorationType(world, DECO_FIRE_PIT);
}

static bool IsMaltaSnakeResolved(const World *world) {
    return IsMaltaFireLit(world) && !HasDecorationType(world, DECO_SNAKE);
}

static int CountRomeBelieversMet(const Player *player) {
    int count = 0;
    for (int i = 0; i < 3; i++) {
        if (player->romeBelieversMet[i]) count++;
    }
    return count;
}

static void HandleRomeNpcInteraction(Player *player, World *world, const char *npcName) {
    if (TextIsEqual(npcName, "Roman Believer 1")) {
        if (!player->romeBelieversMet[0]) {
            player->romeBelieversMet[0] = true;
            player->spirit += 10;
            if (player->spirit > player->maxSpirit) player->spirit = player->maxSpirit;
        }
        ShowWorldMessage(player, "Believer: 'Brother Paul, we came from Rome to meet you. We have prayed for you on the journey.'", 6.0f);
        return;
    }

    if (TextIsEqual(npcName, "Roman Believer 2")) {
        if (!player->romeBelieversMet[1]) {
            player->romeBelieversMet[1] = true;
            player->spirit += 10;
            if (player->spirit > player->maxSpirit) player->spirit = player->maxSpirit;
        }
        ShowWorldMessage(player, "Believer: 'Take courage, Paul. The brothers in Rome thank God for your safe arrival.'", 6.0f);
        return;
    }

    if (TextIsEqual(npcName, "Roman Believer 3")) {
        if (!player->romeBelieversMet[2]) {
            player->romeBelieversMet[2] = true;
            player->spirit += 10;
            if (player->spirit > player->maxSpirit) player->spirit = player->maxSpirit;
        }
        ShowWorldMessage(player, "Believer: 'The Lord has brought you here. Be encouraged, for you will still bear witness in Rome.'", 6.0f);
        return;
    }

    if (TextIsEqual(npcName, "Centurion")) {
        if (CountRomeBelieversMet(player) < 3) {
            ShowWorldMessage(player, "Centurion: 'You may proceed when you have spoken with those who came to greet you on the road.'", 6.0f);
            return;
        }

        player->romeCenturionMet = true;
        player->questStates[4] = QUEST_COMPLETED;
        player->activeQuestId = 0;
        ResetPlayerMovement(player, world->state.houseArrestPos);
        player->gameComplete = true;
    }
}

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

    if (FileExists("assets/viking_boat.glb")) {
        boatModel = LoadModel("assets/viking_boat.glb");
        boatModelLoaded = boatModel.meshCount > 0;
    }

    if (boatModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(boatModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        if (sizeX < 0.001f) sizeX = 1.0f;
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (sizeZ < 0.001f) sizeZ = 1.0f;

        float scaleX = 32.5f / sizeX;
        float scaleY = 16.0f / sizeY;
        float scaleZ = 15.0f / sizeZ;
        float scale = fminf(scaleX, fminf(scaleY, scaleZ));
        boatModelScale = (Vector3){scale, scale, scale};
        boatModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/al_capone.glb")) {
        islanderModel = LoadModel("assets/al_capone.glb");
        islanderModelLoaded = islanderModel.meshCount > 0;
    }

    if (islanderModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(islanderModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxXZ = fmaxf(sizeX, sizeZ);
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (maxXZ < 0.001f) maxXZ = 1.0f;

        float scaleY = 1.9f / sizeY;
        float scaleXZ = 1.4f / maxXZ;
        float scale = fminf(scaleY, scaleXZ);
        islanderModelScale = (Vector3){scale, scale, scale};
        islanderModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/paul.glb")) {
        paulModel = LoadModel("assets/paul.glb");
        paulModelLoaded = paulModel.meshCount > 0;
    }

    if (paulModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(paulModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxXZ = fmaxf(sizeX, sizeZ);
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (maxXZ < 0.001f) maxXZ = 1.0f;

        float scaleY = 1.9f / sizeY;
        float scaleXZ = 1.4f / maxXZ;
        float scale = fminf(scaleY, scaleXZ);
        paulModelScale = (Vector3){scale, scale, scale};
        paulModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/roman_character.glb")) {
        romanCharacterModel = LoadModel("assets/roman_character.glb");
        romanCharacterModelLoaded = romanCharacterModel.meshCount > 0;
    }

    if (romanCharacterModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(romanCharacterModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxXZ = fmaxf(sizeX, sizeZ);
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (maxXZ < 0.001f) maxXZ = 1.0f;

        float scaleY = 1.85f / sizeY;
        float scaleXZ = 1.25f / maxXZ;
        float scale = fminf(scaleY, scaleXZ);
        romanCharacterModelScale = (Vector3){scale, scale, scale};
        romanCharacterModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }

    if (FileExists("assets/roman_soldier.glb")) {
        romanSoldierModel = LoadModel("assets/roman_soldier.glb");
        romanSoldierModelLoaded = romanSoldierModel.meshCount > 0;
    }

    if (romanSoldierModelLoaded) {
        BoundingBox bounds = GetModelBoundingBox(romanSoldierModel);
        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;
        float maxXZ = fmaxf(sizeX, sizeZ);
        if (sizeY < 0.001f) sizeY = 1.0f;
        if (maxXZ < 0.001f) maxXZ = 1.0f;

        float scaleY = 1.95f / sizeY;
        float scaleXZ = 1.25f / maxXZ;
        float scale = fminf(scaleY, scaleXZ);
        romanSoldierModelScale = (Vector3){scale, scale, scale};
        romanSoldierModelOffset = (Vector3){
            -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
            -(bounds.min.y * scale),
            -((bounds.min.z + bounds.max.z) * 0.5f) * scale
        };
    }
}

static void UnloadModels(void) {
    if (snakeModelLoaded) {
        UnloadModel(snakeModel);
        snakeModelLoaded = false;
    }
    if (columnModelLoaded) {
        UnloadModel(columnModel);
        columnModelLoaded = false;
    }
    if (templeModelLoaded) {
        UnloadModel(templeModel);
        templeModelLoaded = false;
    }
    if (desertHouseModelLoaded) {
        UnloadModel(desertHouseModel);
        desertHouseModelLoaded = false;
    }
    if (sadduceeModelLoaded) {
        UnloadModel(sadduceeModel);
        sadduceeModelLoaded = false;
    }
    if (boatModelLoaded) {
        UnloadModel(boatModel);
        boatModelLoaded = false;
    }
    if (islanderModelLoaded) {
        UnloadModel(islanderModel);
        islanderModelLoaded = false;
    }
    if (paulModelLoaded) {
        UnloadModel(paulModel);
        paulModelLoaded = false;
    }
    if (romanCharacterModelLoaded) {
        UnloadModel(romanCharacterModel);
        romanCharacterModelLoaded = false;
    }
    if (romanSoldierModelLoaded) {
        UnloadModel(romanSoldierModel);
        romanSoldierModelLoaded = false;
    }
}

static void MovePlayerToPort(Player *player, World *world, int portIdx) {
    Vector3Int p = world->state.ports[portIdx].position;
    ResetPlayerMovement(player, p);
}

static void HandlePortTravel(Player *player, World *world, ShipMinigame *shipMinigame, int portIdx) {
    if (world->state.nextWorldId == WORLD_NONE) return;
    if (world->state.worldId == WORLD_JUDEA && !player->guardClearedForShip) return;
    StartShipMinigame(shipMinigame, world->state.nextWorldId, world->state.worldName, world->state.nextWorldName);
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

    Image imgLetter = GenImageColor(48, 48, BLANK);
    ImageDrawRectangle(&imgLetter, 8, 12, 32, 22, (Color){244, 232, 196, 255});
    ImageDrawRectangleLines(&imgLetter, (Rectangle){8, 12, 32, 22}, 2, DARKBROWN);
    ImageDrawLine(&imgLetter, 8, 12, 24, 24, DARKBROWN);
    ImageDrawLine(&imgLetter, 40, 12, 24, 24, DARKBROWN);
    ImageDrawCircleV(&imgLetter, (Vector2){34, 28}, 3, RED);
    spriteDatabase[SPRITE_LETTER] = LoadTextureFromImage(imgLetter);
    UnloadImage(imgLetter);
}

static void LoadShaderSettings(ShaderSettings *settings) {
    *settings = (ShaderSettings){
        .lightDir = (Vector3){0.0f, 1.0f, 0.0f},
        .lightColor = WHITE,
        .ambient = 0.6f,
        .shadowBias = 0.005f,
        .showDebugUI = false
    };

    if (SPPS_PLATFORM_WEB) return;

    FILE *f = fopen("shader_settings.txt", "r");
    if (!f) return;

    int r, g, b;
    fscanf(f, "%f %f %f", &settings->lightDir.x, &settings->lightDir.y, &settings->lightDir.z);
    fscanf(f, "%d %d %d", &r, &g, &b);
    settings->lightColor = (Color){(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
    fscanf(f, "%f", &settings->ambient);
    fscanf(f, "%f", &settings->shadowBias);
    fclose(f);
}

static bool BrowserAliasAllowed(const GameRuntime *game) {
    return SPPS_PLATFORM_WEB && !game->combat.active && !game->player.guardDialogueActive;
}

static bool IsActionTogglePressed(const GameRuntime *game, int functionKey, int browserAliasKey) {
    return IsKeyPressed(functionKey) || (BrowserAliasAllowed(game) && IsKeyPressed(browserAliasKey));
}

static void EnterFirstPerson(GameRuntime *game) {
    game->isFirstPerson = true;
    DisableCursor();
}

static void ExitFirstPerson(GameRuntime *game) {
    game->isFirstPerson = false;
    EnableCursor();
}

static void InitializeShadowPipeline(GameRuntime *game) {
    if (SPPS_PLATFORM_WEB) return;

    game->shadowShader = LoadShader("shaders/shadow.vs", "shaders/shadow.fs");
    game->depthShader = LoadShader("shaders/depth.vs", "shaders/depth.fs");
    game->shadowMap = LoadRenderTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

    game->shadowMapLoc = GetShaderLocation(game->shadowShader, "shadowMap");
    game->shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(game->shadowShader, "viewPos");
    game->lightDirLoc = GetShaderLocation(game->shadowShader, "lightDir");
    game->lightColorLoc = GetShaderLocation(game->shadowShader, "lightColor");
    game->ambientLoc = GetShaderLocation(game->shadowShader, "ambient");
    game->shadowBiasLoc = GetShaderLocation(game->shadowShader, "shadowBias");
    game->lightVPLoc = GetShaderLocation(game->shadowShader, "lightVP");

    float smSize = (float)SHADOW_MAP_SIZE;
    int shadowMapSizeLoc = GetShaderLocation(game->shadowShader, "shadowMapSize");
    SetShaderValue(game->shadowShader, shadowMapSizeLoc, &smSize, SHADER_UNIFORM_FLOAT);

    game->shadowPipelineReady = true;
}

static void InitializeGame(GameRuntime *game) {
    LoadShaderSettings(&game->settings);

    InitWindow(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, "St. Paul's Postal Service");
    DrawStartupLoadingScreen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0.08f, "Opening travel logs...");
    LoadSprites();
    DrawStartupLoadingScreen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0.28f, "Loading characters and world models...");
    LoadModels();

    if (SPPS_PLATFORM_WEB) {
        DrawStartupLoadingScreen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0.56f, "Preparing browser-safe rendering...");
    } else {
        DrawStartupLoadingScreen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0.56f, "Preparing lighting and shadows...");
        InitializeShadowPipeline(game);
    }

    DrawStartupLoadingScreen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 0.78f, "Gathering companions and ports...");
    InitPlayer(&game->player);
    InitWorld(&game->world);
    InitShipMinigame(&game->shipMinigame);
    DrawStartupLoadingScreen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT, 1.0f, "Journey ready.");

    game->camera.position = (Vector3){8.0f, 8.0f, 8.0f};
    game->camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    game->camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    game->camera.fovy = 45.0f;
    game->camera.projection = CAMERA_PERSPECTIVE;
    game->shadersEnabled = false;

    SetTargetFPS(60);
}

static void ShutdownGame(GameRuntime *game) {
    if (game->shadowPipelineReady) {
        UnloadShader(game->shadowShader);
        UnloadShader(game->depthShader);
        UnloadRenderTexture(game->shadowMap);
    }

    UnloadShipMinigame();
    UnloadModels();
    CloseWindow();
}

static void UpdateDrawFrame(void *arg) {
    GameRuntime *game = (GameRuntime *)arg;

    if (game->shipMinigame.completed) {
        LoadWorld(&game->world, game->shipMinigame.destinationWorld);
        if (game->world.state.portCount > 0) {
            MovePlayerToPort(&game->player, &game->world, 0);
        }
        InitShipMinigame(&game->shipMinigame);
    } else if (game->shipMinigame.failed) {
        if (game->shipMinigame.resultMessage[0] != '\0') {
            ShowWorldMessage(&game->player, game->shipMinigame.resultMessage, 5.0f);
        } else {
            ShowWorldMessage(&game->player, "The voyage failed. You return to shore and remain in the current port.", 5.0f);
        }
        InitShipMinigame(&game->shipMinigame);
    }

    if (game->player.worldMessageTimer > 0.0f) {
        game->player.worldMessageTimer -= GetFrameTime();
        if (game->player.worldMessageTimer <= 0.0f) {
            game->player.worldMessageTimer = 0.0f;
            game->player.worldMessage[0] = '\0';
        }
    }

    if (game->snakeEventTimer > 0.0f) {
        game->snakeEventTimer -= GetFrameTime();
        if (game->snakeEventTimer <= 0.0f && game->world.state.worldId == WORLD_MALTA) {
            for (int i = 0; i < MAX_DECORATIONS; i++) {
                if (game->world.state.decos[i].type == DECO_SNAKE) {
                    game->world.state.decos[i].type = DECO_NONE;
                }
            }
            ShowWorldMessage(&game->player, "You shake off the creature into the fire and suffer no harm.", 5.0f);
        }
    }

    if (game->shipMinigame.active) {
        UpdateShipMinigame(&game->shipMinigame);
    } else {
        if (IsActionTogglePressed(game, KEY_F1, KEY_ONE)) EnterFirstPerson(game);

        if (IsActionTogglePressed(game, KEY_F2, KEY_TWO)) {
            if (game->shadowPipelineReady) {
                game->shadersEnabled = !game->shadersEnabled;
            } else if (SPPS_PLATFORM_WEB) {
                ShowWorldMessage(&game->player, "Browser build: shadow shaders are disabled for compatibility.", 4.0f);
            }
        }

        if (IsActionTogglePressed(game, KEY_F3, KEY_THREE)) ExitFirstPerson(game);

        if (IsActionTogglePressed(game, KEY_F5, KEY_FIVE)) {
            game->settings.showDebugUI = !game->settings.showDebugUI;
            if (game->settings.showDebugUI) EnableCursor();
        }

        if (game->isFirstPerson) {
            Vector2 delta = GetMouseDelta();
            game->player.yaw -= delta.x * 0.005f;
            game->player.pitch += delta.y * -0.005f;
            if (game->player.pitch > PI / 2.5f) game->player.pitch = PI / 2.5f;
            if (game->player.pitch < -PI / 2.5f) game->player.pitch = -PI / 2.5f;

            game->camera.position = (Vector3){game->player.lerpPosition.x, 1.6f, game->player.lerpPosition.z};
            Vector3 look = {
                cosf(game->player.pitch) * sinf(game->player.yaw),
                sinf(game->player.pitch),
                cosf(game->player.pitch) * cosf(game->player.yaw)
            };
            game->camera.target = Vector3Add(game->camera.position, look);
            game->camera.fovy = 60.0f;
        } else {
            game->camera.target = game->player.lerpPosition;
            game->camera.position = (Vector3){game->player.lerpPosition.x + 8.0f, 8.0f, game->player.lerpPosition.z + 8.0f};
            game->camera.fovy = 45.0f;
        }

        if (!game->settings.showDebugUI && !game->combat.active && !game->player.guardDialogueActive && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = game->isFirstPerson
                ? GetMouseRay((Vector2){(float)GetScreenWidth() / 2, (float)GetScreenHeight() / 2}, game->camera)
                : GetMouseRay(GetMousePosition(), game->camera);

            int npcIdx = GetClickedNPC(&game->world, ray);
            if (npcIdx != -1) {
                const char *npcName = game->world.state.npcs[npcIdx].name;

                if (TextIsEqual(npcName, "Pharisee")) {
                    StartCombat(&game->combat, npcName);
                } else if (TextIsEqual(npcName, "Islander")) {
                    bool nearSnake =
                        abs(game->player.position.x - game->world.state.snakePos.x) <= 6 &&
                        abs(game->player.position.z - game->world.state.snakePos.z) <= 6;
                    if (game->world.state.worldId == WORLD_MALTA && nearSnake && IsMaltaSnakeResolved(&game->world)) {
                        ShowWorldMessage(&game->player, "Islander: 'He must be a god! He suffered no harm!'", 5.0f);
                    } else {
                        ShowWorldMessage(&game->player, "Islander: 'No doubt this man is a murderer, for justice has not allowed him to live.'", 5.0f);
                    }
                } else if (TextIsEqual(npcName, "Roman Believer 1") ||
                           TextIsEqual(npcName, "Roman Believer 2") ||
                           TextIsEqual(npcName, "Roman Believer 3") ||
                           TextIsEqual(npcName, "Centurion")) {
                    HandleRomeNpcInteraction(&game->player, &game->world, npcName);
                } else if (!TextIsEqual(npcName, "Guard")) {
                    ShowWorldMessage(&game->player, TextFormat("%s pauses as you approach.", npcName), 3.0f);
                }
            } else {
                int itemIdx = GetClickedItem(&game->world, ray);
                int decoIdx = GetClickedDecoration(&game->world, ray);

                if (itemIdx != -1) {
                    if (AddToInventory(&game->player, game->world.state.items[itemIdx].itemId)) {
                        game->world.state.items[itemIdx].active = false;
                    }
                } else if (decoIdx != -1) {
                    if (game->world.state.decos[decoIdx].type == DECO_FIRE_PIT_UNLIT) {
                        game->world.state.decos[decoIdx].type = DECO_FIRE_PIT;
                        for (int i = 0; i < MAX_DECORATIONS; i++) {
                            if (game->world.state.decos[i].position.x == game->world.state.snakePos.x &&
                                game->world.state.decos[i].position.z == game->world.state.snakePos.z &&
                                game->world.state.decos[i].type == DECO_NONE) {
                                game->world.state.decos[i].type = DECO_SNAKE;
                                break;
                            }
                        }
                        ShowWorldMessage(&game->player, "A viper fastens on your hand! The islanders watch closely...", 5.0f);
                        game->snakeEventTimer = 8.0f;
                    } else if (game->world.state.decos[decoIdx].type == DECO_FIRE_PIT) {
                        ShowWorldMessage(&game->player, "The fire burns warmly.", 3.0f);
                    }
                } else if (!game->isFirstPerson) {
                    Vector3Int gridClick = GetGridClicked(ray);
                    if (gridClick.y != -1) {
                        FindPath(&game->world, &game->player, gridClick);
                    }
                }
            }
        }

        if (game->combat.active) {
            UpdateCombat(&game->combat, &game->player);
        } else {
            bool travelStarted = false;

            UpdateGuardDialogue(&game->player, &game->world);

            int portIdx = GetPortAt(&game->world, game->player.position);
            if (portIdx != -1 && IsKeyPressed(KEY_T) && !game->player.guardDialogueActive) {
                if (game->world.state.worldId == WORLD_MALTA && !IsMaltaSnakeResolved(&game->world)) {
                    ShowWorldMessage(&game->player, "Captain: 'We must wait for the winter storms to pass.'", 4.0f);
                } else {
                    ExitFirstPerson(game);
                    HandlePortTravel(&game->player, &game->world, &game->shipMinigame, portIdx);
                    travelStarted = game->shipMinigame.active;
                }
            }

            if (!travelStarted) {
                if (!game->player.gameComplete && !game->player.guardDialogueActive) {
                    UpdatePlayer(&game->player, &game->world, game->isFirstPerson);
                }
                UpdateWorld(&game->world, &game->player);

                if (!game->player.guardDialogueActive && IsKeyPressed(KEY_C)) TryCraftTent(&game->player);
                if (!game->player.guardDialogueActive && IsKeyPressed(KEY_I)) game->player.showInventory = !game->player.showInventory;
                if (!game->player.guardDialogueActive && IsKeyPressed(KEY_M)) game->player.showMap = !game->player.showMap;
            }
        }
    }

    BeginDrawing();
    if (game->shipMinigame.active) {
        DrawShipMinigame(&game->shipMinigame, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
        SppsInputFrameEnd();
        return;
    }

    bool drawShadows = game->shadersEnabled && game->shadowPipelineReady;
    Matrix lightProj = {0};
    Matrix lightVP = {0};
    Vector3 lightDir = game->settings.lightDir;
    Vector3 lightUp = {0};
    Vector3 lightCamPos = {0};
    Vector3 center = game->isFirstPerson ? game->player.lerpPosition : game->camera.target;

    if (drawShadows) {
        float shadowBoxSize = 60.0f;
        if (Vector3LengthSqr(lightDir) < 0.0001f) lightDir = (Vector3){0.0f, 1.0f, 0.0f};
        lightDir = Vector3Normalize(lightDir);
        float lightDistance = 40.0f;
        Vector3 lightPos = Vector3Scale(lightDir, lightDistance);
        lightUp = fabsf(Vector3DotProduct(lightDir, (Vector3){0, 1, 0})) > 0.98f
                ? (Vector3){0, 0, 1}
                : (Vector3){0, 1, 0};
        lightCamPos = Vector3Add(center, lightPos);
        Matrix lightView = MatrixLookAt(lightCamPos, center, lightUp);
        lightProj = MatrixOrtho(-shadowBoxSize, shadowBoxSize, -shadowBoxSize, shadowBoxSize, 1.0f, 150.0f);
        lightVP = MatrixMultiply(lightProj, lightView);

        BeginTextureMode(game->shadowMap);
            ClearBackground(WHITE);
            BeginMode3D((Camera3D){lightCamPos, center, lightUp, 90.0f, CAMERA_ORTHOGRAPHIC});
                rlSetMatrixProjection(lightProj);
                BeginShaderMode(game->depthShader);
                    rlDisableBackfaceCulling();
                    DrawWorld(&game->world, (Camera3D){lightCamPos, center, lightUp, 40.0f, CAMERA_ORTHOGRAPHIC}, false);
                    rlEnableBackfaceCulling();
                EndShaderMode();
            EndMode3D();
        EndTextureMode();
    }

    ClearBackground(SKYBLUE);
    BeginMode3D(game->camera);
        if (drawShadows) {
            SetShaderValueMatrix(game->shadowShader, game->lightVPLoc, lightVP);
            SetShaderValue(game->shadowShader, game->shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &game->camera.position, SHADER_UNIFORM_VEC3);
            SetShaderValue(game->shadowShader, game->lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);

            Vector3 lightColorVec = {
                (float)game->settings.lightColor.r / 255.0f,
                (float)game->settings.lightColor.g / 255.0f,
                (float)game->settings.lightColor.b / 255.0f
            };
            SetShaderValue(game->shadowShader, game->lightColorLoc, &lightColorVec, SHADER_UNIFORM_VEC3);
            SetShaderValue(game->shadowShader, game->ambientLoc, &game->settings.ambient, SHADER_UNIFORM_FLOAT);
            SetShaderValue(game->shadowShader, game->shadowBiasLoc, &game->settings.shadowBias, SHADER_UNIFORM_FLOAT);

            rlActiveTextureSlot(1);
            rlEnableTexture(game->shadowMap.texture.id);
            rlActiveTextureSlot(0);

            int slot = 1;
            SetShaderValue(game->shadowShader, game->shadowMapLoc, &slot, SHADER_UNIFORM_INT);
            BeginShaderMode(game->shadowShader);
        }

        DrawWorld(&game->world, game->camera, drawShadows);
        if (!game->isFirstPerson) DrawPlayer(&game->player, game->camera, drawShadows);

        if (drawShadows) {
            EndShaderMode();
            rlActiveTextureSlot(1);
            rlDisableTexture();
            rlActiveTextureSlot(0);
        }

        if (game->world.state.portCount > 0 && game->world.state.nextWorldId != WORLD_NONE) {
            Vector3Int p = game->world.state.ports[0].position;
            Vector3 base = {(float)p.x, 0.0f, (float)p.z};
            DrawCylinder(base, 0.35f, 0.35f, 2.0f, 8, Fade(YELLOW, 0.7f));
            DrawSphere((Vector3){base.x, 2.2f, base.z}, 0.2f, YELLOW);
        }

        if (!game->combat.active &&
            Vector3Distance(game->player.lerpPosition,
                            (Vector3){(float)game->player.target.x, (float)game->player.target.y, (float)game->player.target.z}) > 0.1f) {
            DrawCircle3D((Vector3){(float)game->player.target.x, 0.01f, (float)game->player.target.z},
                         0.5f,
                         (Vector3){1, 0, 0},
                         90.0f,
                         Fade(YELLOW, 0.5f));
        }
    EndMode3D();

    DrawInventory(&game->player);
    DrawSkills(&game->player);
    DrawHUD(&game->player, &game->world, game->isFirstPerson);
    DrawGuardDialogue(&game->player, &game->world);

    if (game->combat.active) DrawCombatUI(&game->combat, GetScreenWidth(), GetScreenHeight());

    DrawText("Point & Click to move across the Mediterranean", 10, GetScreenHeight() - (SPPS_PLATFORM_WEB ? 48 : 45), 15, WHITE);
    if (SPPS_PLATFORM_WEB) {
        DrawText("Browser: F1/F2/F3/F5 also map to 1/2/3/5. Shift still boosts movement and repair.", 10, GetScreenHeight() - 28, 15, Fade(RAYWHITE, 0.88f));
    }
    DrawFPS(GetScreenWidth() - 92, GetScreenHeight() - 28);
    DrawShaderDebugUI(&game->settings);
    EndDrawing();
    SppsInputFrameEnd();
}

int main(void) {
    InitializeGame(&g_game);

#if SPPS_PLATFORM_WEB
    emscripten_set_main_loop_arg(UpdateDrawFrame, &g_game, 0, 1);
#else
    while (!WindowShouldClose()) UpdateDrawFrame(&g_game);
    ShutdownGame(&g_game);
#endif

    return 0;
}
