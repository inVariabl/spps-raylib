#include "ship_minigame.h"

#include <math.h>

static Model shipModel = {0};
static bool shipModelLoaded = false;
static Vector3 shipModelScale = {1.0f, 1.0f, 1.0f};
static Vector3 shipModelOffset = {0};

static Model sailorModel = {0};
static bool sailorModelLoaded = false;
static Vector3 sailorModelScale = {1.0f, 1.0f, 1.0f};
static Vector3 sailorModelOffset = {0};

static void LoadShipMinigameModels(void) {
    if (!shipModelLoaded && FileExists("assets/roman_boat.glb")) {
        shipModel = LoadModel("assets/roman_boat.glb");
        shipModelLoaded = shipModel.meshCount > 0;
        if (shipModelLoaded) {
            BoundingBox bounds = GetModelBoundingBox(shipModel);
            float sizeX = bounds.max.x - bounds.min.x;
            float sizeY = bounds.max.y - bounds.min.y;
            float sizeZ = bounds.max.z - bounds.min.z;
            float scale = 1.0f / fmaxf(0.001f, fmaxf(sizeX, fmaxf(sizeY, sizeZ)));
            shipModelScale = (Vector3){scale * 8.0f, scale * 8.0f, scale * 8.0f};
            shipModelOffset = (Vector3){
                -((bounds.min.x + bounds.max.x) * 0.5f) * shipModelScale.x,
                -(bounds.min.y * shipModelScale.y),
                -((bounds.min.z + bounds.max.z) * 0.5f) * shipModelScale.z
            };
        }
    }

    if (!sailorModelLoaded && FileExists("assets/roman_character.glb")) {
        sailorModel = LoadModel("assets/roman_character.glb");
        sailorModelLoaded = sailorModel.meshCount > 0;
        if (sailorModelLoaded) {
            BoundingBox bounds = GetModelBoundingBox(sailorModel);
            float sizeX = bounds.max.x - bounds.min.x;
            float sizeY = bounds.max.y - bounds.min.y;
            float sizeZ = bounds.max.z - bounds.min.z;
            float scaleY = 1.4f / fmaxf(0.001f, sizeY);
            float scaleXZ = 0.9f / fmaxf(0.001f, fmaxf(sizeX, sizeZ));
            float scale = fminf(scaleY, scaleXZ);
            sailorModelScale = (Vector3){scale, scale, scale};
            sailorModelOffset = (Vector3){
                -((bounds.min.x + bounds.max.x) * 0.5f) * scale,
                -(bounds.min.y * scale),
                -((bounds.min.z + bounds.max.z) * 0.5f) * scale
            };
        }
    }
}

void InitShipMinigame(ShipMinigame *game) {
    game->active = false;
    game->failed = false;
    game->completed = false;
    game->travelProgress = 0.0f;
    game->boatX = 0.0f;
    game->bobTimer = 0.0f;
    game->hullPoints = 3;
    game->destinationWorld = WORLD_NONE;
    game->fromName = "";
    game->toName = "";
    game->spawnTimer = 0.0f;
    for (int i = 0; i < SHIP_GAME_OBSTACLE_COUNT; i++) {
        game->obstacleActive[i] = false;
        game->obstacleRadius[i] = 0.8f;
        game->obstacleCoin[i] = false;
        game->obstaclePos[i] = (Vector3){0};
    }
}

static void ResetVoyageState(ShipMinigame *game) {
    game->failed = false;
    game->completed = false;
    game->travelProgress = 0.0f;
    game->boatX = 0.0f;
    game->bobTimer = 0.0f;
    game->hullPoints = 3;
    game->spawnTimer = 0.35f;
    for (int i = 0; i < SHIP_GAME_OBSTACLE_COUNT; i++) {
        game->obstacleActive[i] = false;
        game->obstacleCoin[i] = false;
    }
}

void StartShipMinigame(ShipMinigame *game, WorldId destinationWorld, const char *fromName, const char *toName) {
    LoadShipMinigameModels();
    InitShipMinigame(game);
    game->active = true;
    game->destinationWorld = destinationWorld;
    game->fromName = fromName;
    game->toName = toName;
    ResetVoyageState(game);
}

static void SpawnObstacle(ShipMinigame *game) {
    for (int i = 0; i < SHIP_GAME_OBSTACLE_COUNT; i++) {
        if (game->obstacleActive[i]) continue;

        int lane = GetRandomValue(-2, 2);
        game->obstacleActive[i] = true;
        game->obstacleCoin[i] = (GetRandomValue(0, 4) == 0);
        game->obstacleRadius[i] = game->obstacleCoin[i] ? 0.55f : 0.9f;
        game->obstaclePos[i] = (Vector3){lane * 2.4f, 0.35f, 30.0f + (float)GetRandomValue(0, 6)};
        return;
    }
}

void UpdateShipMinigame(ShipMinigame *game) {
    if (!game->active) return;

    float dt = GetFrameTime();

    if (game->failed) {
        if (IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            ResetVoyageState(game);
        }
        return;
    }

    if (game->completed) return;

    game->bobTimer += dt;

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) game->boatX -= 2.4f;
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) game->boatX += 2.4f;
    if (game->boatX < -4.8f) game->boatX = -4.8f;
    if (game->boatX > 4.8f) game->boatX = 4.8f;

    game->spawnTimer -= dt;
    if (game->spawnTimer <= 0.0f) {
        SpawnObstacle(game);
        game->spawnTimer = 0.55f - game->travelProgress * 0.2f;
        if (game->spawnTimer < 0.22f) game->spawnTimer = 0.22f;
    }

    float speed = 11.0f + game->travelProgress * 7.0f;
    for (int i = 0; i < SHIP_GAME_OBSTACLE_COUNT; i++) {
        if (!game->obstacleActive[i]) continue;

        game->obstaclePos[i].z -= speed * dt;
        if (game->obstacleCoin[i]) {
            game->obstaclePos[i].y = 0.8f + sinf(game->bobTimer * 5.0f + i) * 0.25f;
        }

        float dx = game->obstaclePos[i].x - game->boatX;
        float dz = game->obstaclePos[i].z - 2.0f;
        if (fabsf(dx) < 1.5f && fabsf(dz) < 1.8f) {
            if (game->obstacleCoin[i]) {
                game->travelProgress += 0.08f;
            } else {
                game->hullPoints--;
                if (game->hullPoints <= 0) {
                    game->failed = true;
                }
            }
            game->obstacleActive[i] = false;
            continue;
        }

        if (game->obstaclePos[i].z < -6.0f) {
            game->obstacleActive[i] = false;
        }
    }

    game->travelProgress += dt * 0.11f;
    if (game->travelProgress >= 1.0f) {
        game->travelProgress = 1.0f;
        game->completed = true;
        game->active = false;
    }
}

void DrawShipMinigame(const ShipMinigame *game, int screenWidth, int screenHeight) {
    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 13.0f, -13.5f};
    camera.target = (Vector3){0.0f, 0.5f, 10.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 42.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    ClearBackground((Color){125, 185, 220, 255});
    BeginMode3D(camera);

    DrawPlane((Vector3){0.0f, -0.15f, 10.0f}, (Vector2){24.0f, 80.0f}, (Color){32, 105, 168, 255});
    for (int i = 0; i < 16; i++) {
        float z = -6.0f + i * 5.0f + fmodf(game->bobTimer * 8.0f, 5.0f);
        DrawCube((Vector3){0.0f, -0.02f, z}, 24.0f, 0.03f, 1.1f, (Color){64, 136, 196, 255});
    }

    for (int lane = -2; lane <= 2; lane++) {
        DrawLine3D((Vector3){lane * 2.4f, 0.0f, -8.0f}, (Vector3){lane * 2.4f, 0.0f, 34.0f}, Fade(RAYWHITE, 0.15f));
    }

    Vector3 boatPos = {game->boatX, 0.1f + sinf(game->bobTimer * 3.0f) * 0.08f, 2.0f};
    if (shipModelLoaded) {
        DrawModelEx(shipModel, Vector3Add(boatPos, shipModelOffset), (Vector3){0.0f, 1.0f, 0.0f}, 180.0f, shipModelScale, WHITE);
    } else {
        DrawCube((Vector3){boatPos.x, boatPos.y + 0.3f, boatPos.z}, 2.8f, 0.6f, 5.0f, BROWN);
        DrawCube((Vector3){boatPos.x, boatPos.y + 1.1f, boatPos.z + 0.2f}, 0.2f, 1.6f, 0.2f, DARKBROWN);
        DrawCube((Vector3){boatPos.x + 0.8f, boatPos.y + 1.6f, boatPos.z + 0.2f}, 1.8f, 1.0f, 0.08f, BEIGE);
    }

    if (sailorModelLoaded) {
        Vector3 sailorPos = {boatPos.x, boatPos.y + 0.5f, boatPos.z - 0.2f};
        DrawModelEx(sailorModel, Vector3Add(sailorPos, sailorModelOffset), (Vector3){0.0f, 1.0f, 0.0f}, 180.0f, sailorModelScale, WHITE);
    }

    for (int i = 0; i < SHIP_GAME_OBSTACLE_COUNT; i++) {
        if (!game->obstacleActive[i]) continue;
        if (game->obstacleCoin[i]) {
            DrawSphere(game->obstaclePos[i], game->obstacleRadius[i], GOLD);
            DrawCircle3D(game->obstaclePos[i], 0.42f, (Vector3){0.0f, 1.0f, 0.0f}, 90.0f, YELLOW);
            DrawCircle3D(game->obstaclePos[i], 0.42f, (Vector3){1.0f, 0.0f, 0.0f}, 90.0f, Fade(YELLOW, 0.65f));
        } else {
            DrawSphere(game->obstaclePos[i], game->obstacleRadius[i], DARKGRAY);
        }
    }
    EndMode3D();

    DrawRectangle(0, 0, screenWidth, 92, Fade(BLACK, 0.35f));
    DrawText(TextFormat("Voyage: %s to %s", game->fromName, game->toName), 24, 18, 28, RAYWHITE);
    DrawText("Steer with A/D or arrow keys. Survive the crossing to continue.", 24, 52, 18, SKYBLUE);

    int barWidth = 300;
    DrawRectangle(screenWidth - barWidth - 28, 24, barWidth, 18, Fade(BLACK, 0.6f));
    DrawRectangle(screenWidth - barWidth - 26, 26, (int)((barWidth - 4) * game->travelProgress), 14, GOLD);
    DrawText("Voyage", screenWidth - barWidth - 28, 48, 16, RAYWHITE);

    for (int i = 0; i < 3; i++) {
        Color hullColor = (i < game->hullPoints) ? RED : Fade(LIGHTGRAY, 0.35f);
        DrawCircle(screenWidth - 30 - i * 24, 76, 9, hullColor);
    }

    if (game->failed) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.55f));
        DrawText("The ship was battered by the storm.", screenWidth / 2 - 220, screenHeight / 2 - 20, 32, GOLD);
        DrawText("Press R, ENTER, or SPACE to attempt the voyage again.", screenWidth / 2 - 260, screenHeight / 2 + 26, 22, RAYWHITE);
    }
}

void UnloadShipMinigame(void) {
    if (shipModelLoaded) {
        UnloadModel(shipModel);
        shipModelLoaded = false;
    }
    if (sailorModelLoaded) {
        UnloadModel(sailorModel);
        sailorModelLoaded = false;
    }
}
