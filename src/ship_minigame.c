#include "ship_minigame.h"

#include "collision.h"
#include "crew.h"
#include "game.h"
#include "interior.h"
#include "objects.h"
#include "ocean.h"
#include "spawner.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    bool assetsLoaded;
    bool ownsAudioDevice;
    Camera3D camera;
    GameObject boat;
    GameObject island;
    GameObject rocks[MAX_ROCKS];
    GameObject gold[MAX_GOLD];
    Vector3 islandScale;
    CrewMember crew[MAX_CREW];
    InteriorState interior;
    int boatHealth;
    int collectedBars;
    float waveTime;
    bool reachedMalta;
    bool moraleGameOver;
    int rockDistanceCounter;
    int goldDistanceCounter;
    int rockDistanceSeparation;
    int goldDistanceSeparation;
    Music musicDeck;
    Music musicInterior;
    Sound crash;
    Sound goldSound;
    bool interiorMusicPlaying;
} VoyageRuntime;

static VoyageRuntime s_runtime = {0};

static bool AnyContinueKeyPressed(void) {
    return IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE);
}

static void ResetInteriorState(InteriorState *interior) {
    interior->paulPos = (Vector3){0.0f, 1.6f, INTERIOR_ROOM_D * 0.4f};
    interior->paulYaw = 180.0f;
    interior->fullscreen = false;
    interior->camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    interior->camera.fovy = 70.0f;
    interior->camera.projection = CAMERA_PERSPECTIVE;
}

static void StopVoyageAudio(void) {
    if (IsMusicValid(s_runtime.musicDeck)) StopMusicStream(s_runtime.musicDeck);
    if (IsMusicValid(s_runtime.musicInterior)) StopMusicStream(s_runtime.musicInterior);
    s_runtime.interiorMusicPlaying = false;
}

static void StartDeckMusic(void) {
    StopVoyageAudio();
    if (IsMusicValid(s_runtime.musicDeck)) PlayMusicStream(s_runtime.musicDeck);
}

static bool EnsureVoyageAssetsLoaded(char *resultMessage, size_t resultMessageSize) {
    if (s_runtime.assetsLoaded) return true;

    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
        s_runtime.ownsAudioDevice = true;
    }

    s_runtime.boat = CreateGameObject("spps-voyage/models/boat.glb", (Vector3){0.0f, 0.0f, 0.0f});
    if (!s_runtime.boat.active) {
        snprintf(resultMessage, resultMessageSize, "%s", "Voyage boat model failed to load.");
        return false;
    }

    s_runtime.island = CreateGameObject(
        "spps-voyage/models/malta_island.glb",
        (Vector3){FINISH_LINE_X - ISLAND_OFFSET, 0.0f, 0.0f}
    );
    s_runtime.islandScale = (Vector3){6.0f, 6.0f, 20.0f};

    for (int i = 0; i < MAX_ROCKS; i++) {
        float rx = (float)GetRandomValue(-300, -20);
        float rz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        s_runtime.rocks[i] = CreateGameObject("spps-voyage/models/rock.glb", (Vector3){rx, 0.0f, rz});
    }

    for (int i = 0; i < MAX_GOLD; i++) {
        float gx = (float)GetRandomValue(-280, -40);
        float gz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        s_runtime.gold[i] = CreateGameObject("spps-voyage/models/gold_coin.glb", (Vector3){gx, 1.5f, gz});
    }

    InitCrewModel();
    InitCrew(s_runtime.crew, MAX_CREW);
    InitInterior(&s_runtime.interior);
    ResetInteriorState(&s_runtime.interior);

    s_runtime.musicDeck = LoadMusicStream("spps-voyage/audio/ocean_sounds.wav");
    s_runtime.musicInterior = LoadMusicStream("spps-voyage/audio/interior.wav");
    s_runtime.crash = LoadSound("spps-voyage/audio/crash.wav");
    s_runtime.goldSound = LoadSound("spps-voyage/audio/gold.wav");

    s_runtime.assetsLoaded = true;
    return true;
}

static void ResetVoyageRun(void) {
    s_runtime.camera = (Camera3D){0};
    s_runtime.camera.position = (Vector3){20.0f, 15.0f, 0.0f};
    s_runtime.camera.target = (Vector3){0.0f, 5.0f, 0.0f};
    s_runtime.camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    s_runtime.camera.fovy = 50.0f;
    s_runtime.camera.projection = CAMERA_PERSPECTIVE;

    s_runtime.boat.position = (Vector3){0.0f, 0.0f, 0.0f};
    s_runtime.boat.active = IsModelValid(s_runtime.boat.model);
    s_runtime.island.position = (Vector3){FINISH_LINE_X - ISLAND_OFFSET, 0.0f, 0.0f};
    s_runtime.island.active = IsModelValid(s_runtime.island.model);

    for (int i = 0; i < MAX_ROCKS; i++) {
        float rx = (float)GetRandomValue(-300, -20);
        float rz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        s_runtime.rocks[i].position = (Vector3){rx, 0.0f, rz};
        s_runtime.rocks[i].active = IsModelValid(s_runtime.rocks[i].model);
    }

    for (int i = 0; i < MAX_GOLD; i++) {
        float gx = (float)GetRandomValue(-280, -40);
        float gz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        s_runtime.gold[i].position = (Vector3){gx, 1.5f, gz};
        s_runtime.gold[i].active = IsModelValid(s_runtime.gold[i].model);
    }

    InitCrew(s_runtime.crew, MAX_CREW);
    ResetInteriorState(&s_runtime.interior);

    s_runtime.boatHealth = 100;
    s_runtime.collectedBars = 0;
    s_runtime.waveTime = 0.0f;
    s_runtime.reachedMalta = false;
    s_runtime.moraleGameOver = false;
    s_runtime.rockDistanceCounter = 0;
    s_runtime.goldDistanceCounter = 0;
    s_runtime.rockDistanceSeparation = 30;
    s_runtime.goldDistanceSeparation = 30;
    s_runtime.interiorMusicPlaying = false;
    StartDeckMusic();
}

static void UpdateVoyagePlay(void) {
    float dt = GetFrameTime();
    s_runtime.waveTime += dt;

    bool wasFullscreen = s_runtime.interior.fullscreen;

    int nearestCrewIdx = -1;
    UpdateInterior(&s_runtime.interior, s_runtime.crew, MAX_CREW, dt, &nearestCrewIdx);

    if (s_runtime.interior.fullscreen != wasFullscreen) {
        if (s_runtime.interior.fullscreen) {
            if (IsMusicValid(s_runtime.musicDeck)) StopMusicStream(s_runtime.musicDeck);
            if (IsMusicValid(s_runtime.musicInterior)) PlayMusicStream(s_runtime.musicInterior);
            s_runtime.interiorMusicPlaying = true;
        } else {
            if (IsMusicValid(s_runtime.musicInterior)) StopMusicStream(s_runtime.musicInterior);
            if (IsMusicValid(s_runtime.musicDeck)) PlayMusicStream(s_runtime.musicDeck);
            s_runtime.interiorMusicPlaying = false;
        }
    }

    if (s_runtime.interiorMusicPlaying) {
        if (IsMusicValid(s_runtime.musicInterior)) UpdateMusicStream(s_runtime.musicInterior);
    } else {
        if (IsMusicValid(s_runtime.musicDeck)) UpdateMusicStream(s_runtime.musicDeck);
    }

    UpdateCrew(s_runtime.crew, MAX_CREW, dt);

    if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        s_runtime.boatHealth += (int)roundf(18.0f * dt);
        if (s_runtime.boatHealth > 100) s_runtime.boatHealth = 100;

        for (int i = 0; i < MAX_CREW; i++) {
            s_runtime.crew[i].morale += 30.0f * dt;
            if (s_runtime.crew[i].morale > CREW_MORALE_MAX) {
                s_runtime.crew[i].morale = CREW_MORALE_MAX;
            }
            s_runtime.crew[i].boostedRecently = true;
            s_runtime.crew[i].boostTimer = 0.12f;
        }
    }

    float overallMorale = GetOverallMorale(s_runtime.crew, MAX_CREW);
    if (overallMorale <= MORALE_GAMEOVER_VAL) s_runtime.moraleGameOver = true;

    s_runtime.boat.position.x -= 20.0f * dt;
    if (s_runtime.boat.position.x < FINISH_LINE_X) s_runtime.boat.position.x = FINISH_LINE_X;

    if (IsKeyDown(KEY_LEFT)) s_runtime.boat.position.z += 30.0f * dt;
    if (IsKeyDown(KEY_RIGHT)) s_runtime.boat.position.z -= 30.0f * dt;
    if (s_runtime.boat.position.z > BOAT_Z_MAX) s_runtime.boat.position.z = BOAT_Z_MAX;
    if (s_runtime.boat.position.z < BOAT_Z_MIN) s_runtime.boat.position.z = BOAT_Z_MIN;

    s_runtime.boat.position.y = WaveHeight(s_runtime.boat.position.x, s_runtime.boat.position.z, s_runtime.waveTime) * 0.5f;

    s_runtime.camera.position = (Vector3){
        s_runtime.boat.position.x + 20.0f,
        s_runtime.boat.position.y + 10.0f,
        s_runtime.boat.position.z
    };
    s_runtime.camera.target = (Vector3){
        s_runtime.boat.position.x - 60.0f,
        s_runtime.boat.position.y + 2.0f,
        s_runtime.boat.position.z
    };

    if (s_runtime.boat.position.x <= FINISH_LINE_X) s_runtime.reachedMalta = true;

    UpdateRockSpawner(
        s_runtime.rocks,
        MAX_ROCKS,
        s_runtime.boat.position.x,
        &s_runtime.rockDistanceCounter,
        s_runtime.rockDistanceSeparation
    );
    UpdateGoldSpawner(
        s_runtime.gold,
        MAX_GOLD,
        s_runtime.boat.position.x,
        &s_runtime.goldDistanceCounter,
        s_runtime.goldDistanceSeparation
    );

    s_runtime.boatHealth -= CheckRockCollisions(&s_runtime.boat, s_runtime.rocks, MAX_ROCKS, s_runtime.crash);
    CheckGoldCollisions(&s_runtime.boat, s_runtime.gold, MAX_GOLD, s_runtime.goldSound, &s_runtime.collectedBars);
}

static void DrawVoyageWorld(void) {
    float distToMalta = s_runtime.boat.position.x - FINISH_LINE_X;
    float overallMorale = GetOverallMorale(s_runtime.crew, MAX_CREW);
    if (distToMalta < 0.0f) distToMalta = 0.0f;

    RenderInteriorToTexture(&s_runtime.interior, s_runtime.crew, MAX_CREW);

    BeginTextureMode(s_runtime.interior.oceanMiniTex);
        ClearBackground((Color){30, 90, 160, 255});
        BeginMode3D(s_runtime.camera);
            DrawOcean(s_runtime.boat.position.x, s_runtime.waveTime);
            DrawModelEx(s_runtime.boat.model, s_runtime.boat.position,
                        (Vector3){0.0f, 1.0f, 0.0f}, 90.0f,
                        (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
            for (int i = 0; i < MAX_ROCKS; i++) {
                if (s_runtime.rocks[i].active) DrawModel(s_runtime.rocks[i].model, s_runtime.rocks[i].position, 1.0f, WHITE);
            }
            for (int i = 0; i < MAX_GOLD; i++) {
                if (s_runtime.gold[i].active) {
                    DrawModelEx(s_runtime.gold[i].model, s_runtime.gold[i].position,
                                (Vector3){0.0f, 1.0f, 0.0f}, s_runtime.waveTime * 90.0f,
                                (Vector3){3.0f, 3.0f, 3.0f}, GOLD);
                }
            }
            if (s_runtime.island.active) {
                DrawModelEx(s_runtime.island.model, s_runtime.island.position,
                            (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, s_runtime.islandScale, WHITE);
            }
        EndMode3D();
    EndTextureMode();

    if (s_runtime.interior.fullscreen) {
        DrawInteriorHUD(&s_runtime.interior, s_runtime.crew, MAX_CREW,
                        s_runtime.boatHealth, overallMorale,
                        s_runtime.collectedBars, distToMalta);
        return;
    }

    ClearBackground((Color){30, 90, 160, 255});
    DrawRectangle(0, GetScreenHeight() / 2 - 40, GetScreenWidth(), 80, (Color){20, 70, 130, 255});

    BeginMode3D(s_runtime.camera);
        DrawOcean(s_runtime.boat.position.x, s_runtime.waveTime);
        DrawModelEx(s_runtime.boat.model, s_runtime.boat.position,
                    (Vector3){0.0f, 1.0f, 0.0f}, 90.0f,
                    (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
        for (int i = 0; i < MAX_ROCKS; i++) {
            if (s_runtime.rocks[i].active) DrawModel(s_runtime.rocks[i].model, s_runtime.rocks[i].position, 1.0f, WHITE);
        }
        for (int i = 0; i < MAX_GOLD; i++) {
            if (s_runtime.gold[i].active) {
                DrawModelEx(s_runtime.gold[i].model, s_runtime.gold[i].position,
                            (Vector3){0.0f, 1.0f, 0.0f}, s_runtime.waveTime * 90.0f,
                            (Vector3){3.0f, 3.0f, 3.0f}, GOLD);
            }
        }
        if (s_runtime.island.active) {
            DrawModelEx(s_runtime.island.model, s_runtime.island.position,
                        (Vector3){0.0f, 1.0f, 0.0f}, 0.0f, s_runtime.islandScale, WHITE);
        }
    EndMode3D();

    int hudX = 20;
    int barW = 220;
    int barH = 24;
    int lineH = 38;
    int y = 20;

    DrawRectangle(hudX - 4, y - 4, barW + 8, 32, Fade(BLACK, 0.55f));
    DrawRectangle(hudX, y, barW, barH, Fade(DARKGRAY, 0.92f));
    DrawRectangle(hudX, y, (int)((float)barW * ((float)s_runtime.boatHealth / 100.0f)), barH, (Color){200, 40, 40, 255});
    DrawText(TextFormat("Hull: %d", s_runtime.boatHealth), hudX + 8, y + 3, 18, WHITE);
    y += lineH;

    Color moraleColor = (overallMorale > 50.0f) ? (Color){60, 200, 80, 255} : (overallMorale > 25.0f) ? ORANGE : RED;
    DrawRectangle(hudX - 4, y - 4, barW + 8, 32, Fade(BLACK, 0.55f));
    DrawRectangle(hudX, y, barW, barH, Fade(DARKGRAY, 0.92f));
    DrawRectangle(hudX, y, (int)((float)barW * (overallMorale / CREW_MORALE_MAX)), barH, moraleColor);
    DrawText(TextFormat("Morale: %d%%", (int)overallMorale), hudX + 8, y + 3, 18, WHITE);
    y += lineH;

    DrawText(TextFormat("Gold: %d", s_runtime.collectedBars), hudX, y, 24, GOLD);
    y += lineH;
    DrawText(TextFormat("Malta: %.0f m", distToMalta), hudX, y, 22, WHITE);

    DrawInteriorHUD(&s_runtime.interior, s_runtime.crew, MAX_CREW,
                    s_runtime.boatHealth, overallMorale,
                    s_runtime.collectedBars, distToMalta);

    DrawRectangle(0, GetScreenHeight() - 44, GetScreenWidth(), 44, Fade(BLACK, 0.55f));
    DrawText("Arrow keys steer the ship. TAB enters the hold. W/S and A/D move Paul below deck.", 18, GetScreenHeight() - 30, 20, RAYWHITE);
}

void InitShipMinigame(ShipMinigame *game) {
    game->active = false;
    game->failed = false;
    game->completed = false;
    game->destinationWorld = WORLD_NONE;
    game->fromName = "";
    game->toName = "";
    game->phase = SHIP_MINIGAME_IDLE;
    game->phaseTimer = 0.0f;
    game->resultMessage[0] = '\0';
}

void StartShipMinigame(ShipMinigame *game, WorldId destinationWorld, const char *fromName, const char *toName) {
    InitShipMinigame(game);
    game->active = true;
    game->destinationWorld = destinationWorld;
    game->fromName = fromName;
    game->toName = toName;
    game->phase = SHIP_MINIGAME_LOADING;
}

void UpdateShipMinigame(ShipMinigame *game) {
    if (!game->active) return;

    float dt = GetFrameTime();

    if (game->phase == SHIP_MINIGAME_LOADING) {
        game->phaseTimer += dt;
        if (game->phaseTimer >= 0.15f) {
            if (!EnsureVoyageAssetsLoaded(game->resultMessage, sizeof(game->resultMessage))) {
                game->active = false;
                game->failed = true;
                return;
            }
            ResetVoyageRun();
            game->phase = SHIP_MINIGAME_PLAYING;
            game->phaseTimer = 0.0f;
        }
        return;
    }

    if (game->phase == SHIP_MINIGAME_PLAYING) {
        UpdateVoyagePlay();

        if (s_runtime.boatHealth <= 0) {
            StopVoyageAudio();
            snprintf(game->resultMessage, sizeof(game->resultMessage), "%s", "The ship was battered apart before reaching Malta.");
            game->phase = SHIP_MINIGAME_RETURNING;
            game->phaseTimer = 0.0f;
        } else if (s_runtime.moraleGameOver) {
            StopVoyageAudio();
            snprintf(game->resultMessage, sizeof(game->resultMessage), "%s", "The crew lost all hope before landfall.");
            game->phase = SHIP_MINIGAME_RETURNING;
            game->phaseTimer = 0.0f;
        } else if (s_runtime.reachedMalta) {
            StopVoyageAudio();
            snprintf(game->resultMessage, sizeof(game->resultMessage), "%s", "Landfall. The storm has carried you safely to Malta.");
            game->phase = SHIP_MINIGAME_RETURNING;
            game->phaseTimer = 0.0f;
        }
        return;
    }

    if (game->phase == SHIP_MINIGAME_RETURNING) {
        game->phaseTimer += dt;
        if (game->phaseTimer >= 1.5f || AnyContinueKeyPressed()) {
            game->active = false;
            game->completed = s_runtime.reachedMalta;
            game->failed = !s_runtime.reachedMalta;
            game->phase = SHIP_MINIGAME_IDLE;
        }
    }
}

void DrawShipMinigame(const ShipMinigame *game, int screenWidth, int screenHeight) {
    if (game->phase == SHIP_MINIGAME_LOADING) {
        float progress = game->phaseTimer / 0.15f;
        if (progress > 1.0f) progress = 1.0f;

        ClearBackground((Color){8, 28, 54, 255});
        DrawRectangleGradientV(0, 0, screenWidth, screenHeight, (Color){16, 44, 76, 255}, (Color){122, 84, 46, 255});
        DrawCircle(screenWidth - 160, 120, 54, Fade(GOLD, 0.85f));
        DrawRectangle(0, screenHeight - 180, screenWidth, 180, (Color){14, 49, 89, 255});
        DrawTriangle((Vector2){120, screenHeight - 110}, (Vector2){250, screenHeight - 150}, (Vector2){300, screenHeight - 90}, (Color){92, 59, 26, 255});
        DrawTriangle((Vector2){220, screenHeight - 208}, (Vector2){220, screenHeight - 92}, (Vector2){320, screenHeight - 128}, Fade(RAYWHITE, 0.8f));

        const char *title = TextFormat("Preparing Voyage: %s to %s", game->fromName, game->toName);
        DrawText(title, 70, 96, 40, RAYWHITE);
        DrawText("Keeping the same window alive and loading the crossing...", 70, 146, 24, Fade(RAYWHITE, 0.85f));

        int barWidth = 520;
        int barHeight = 20;
        int barX = 70;
        int barY = 208;
        DrawRectangle(barX, barY, barWidth, barHeight, Fade(BLACK, 0.5f));
        DrawRectangle(barX + 3, barY + 3, (int)((float)(barWidth - 6) * progress), barHeight - 6, GOLD);
        DrawRectangleLines(barX, barY, barWidth, barHeight, Fade(RAYWHITE, 0.6f));
        return;
    }

    DrawVoyageWorld();

    if (game->phase == SHIP_MINIGAME_RETURNING) {
        const bool success = s_runtime.reachedMalta;
        const char *headline = success ? "MALTA IN SIGHT" : "VOYAGE FAILED";
        Color headlineColor = success ? GREEN : ORANGE;

        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.56f));
        DrawRectangle(screenWidth / 2 - 280, screenHeight / 2 - 120, 560, 220, Fade((Color){10, 18, 28, 255}, 0.96f));
        DrawRectangleLines(screenWidth / 2 - 280, screenHeight / 2 - 120, 560, 220, Fade(headlineColor, 0.8f));
        DrawText(headline, screenWidth / 2 - MeasureText(headline, 42) / 2, screenHeight / 2 - 72, 42, headlineColor);
        DrawText(game->resultMessage, screenWidth / 2 - MeasureText(game->resultMessage, 22) / 2, screenHeight / 2 - 12, 22, RAYWHITE);
        DrawText("Returning to the main journey...", screenWidth / 2 - 150, screenHeight / 2 + 42, 22, LIGHTGRAY);
        DrawText("Press ENTER, SPACE, or ESC to continue immediately.", screenWidth / 2 - 230, screenHeight / 2 + 78, 18, Fade(RAYWHITE, 0.85f));
    }
}

void UnloadShipMinigame(void) {
    if (!s_runtime.assetsLoaded) return;

    StopVoyageAudio();

    UnloadInterior(&s_runtime.interior);
    UnloadCrewModel();
    UnloadGameObject(&s_runtime.boat);
    UnloadGameObject(&s_runtime.island);
    for (int i = 0; i < MAX_ROCKS; i++) UnloadGameObject(&s_runtime.rocks[i]);
    for (int i = 0; i < MAX_GOLD; i++) UnloadGameObject(&s_runtime.gold[i]);

    if (IsMusicValid(s_runtime.musicDeck)) UnloadMusicStream(s_runtime.musicDeck);
    if (IsMusicValid(s_runtime.musicInterior)) UnloadMusicStream(s_runtime.musicInterior);
    if (IsSoundValid(s_runtime.crash)) UnloadSound(s_runtime.crash);
    if (IsSoundValid(s_runtime.goldSound)) UnloadSound(s_runtime.goldSound);

    if (s_runtime.ownsAudioDevice && IsAudioDeviceReady()) CloseAudioDevice();

    memset(&s_runtime, 0, sizeof(s_runtime));
}
