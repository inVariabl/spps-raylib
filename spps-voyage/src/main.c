// mingw32-make
// .\game.exe

#include "game.h"
#include "objects.h"
#include "ocean.h"
#include "spawner.h"
#include "platform_input.h"
#include "collision.h"
#include "crew.h"
#include "interior.h"

// ----------------------------------------------------------------
// HUD bar helper
// ----------------------------------------------------------------
static void DrawBar(int x, int y, int w, int h,
                    float value, float maxVal,
                    Color fillColor, const char* label)
{
    DrawRectangle(x - 2, y - 2, w + 4, h + 4, BLACK);
    DrawRectangle(x,     y,     w,     h,      DARKGRAY);
    int fillW = (int)(value / maxVal * (float)w);
    if (fillW > 0) DrawRectangle(x, y, fillW, h, fillColor);
    DrawText(label, x + 6, y + (h/2) - 9, 18, WHITE);
}

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "Voyage");
    InitAudioDevice();
    SetTargetFPS(60);

    // ----------------------------------------------------------------
    // Ocean camera
    // ----------------------------------------------------------------
    Camera3D camera   = { 0 };
    camera.position   = (Vector3){ 20.0f, 15.0f, 0.0f };
    camera.target     = (Vector3){  0.0f,  5.0f,  0.0f };
    camera.up         = (Vector3){  0.0f,  1.0f,  0.0f };
    camera.fovy       = 50.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // ----------------------------------------------------------------
    // Models
    // ----------------------------------------------------------------
    GameObject boat = CreateGameObject("spps-voyage/models/boat.glb", (Vector3){ 0, 0, 0 });
    if (!boat.active)
    {
        TraceLog(LOG_ERROR, "GAME: Failed to load boat model, exiting.");
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    GameObject island = CreateGameObject("spps-voyage/models/malta_island.glb",
                                          (Vector3){ FINISH_LINE_X - ISLAND_OFFSET, 0.0f, 0.0f });
    Vector3 islandScale = { 6.0f, 6.0f, 20.0f };

    GameObject rocks[MAX_ROCKS] = { 0 };
    GameObject gold[MAX_GOLD]   = { 0 };

    for (int i = 0; i < MAX_ROCKS; i++)
    {
        float rx = (float)GetRandomValue(-300, -20);
        float rz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        rocks[i] = CreateGameObject("spps-voyage/models/rock.glb", (Vector3){ rx, 0, rz });
    }
    for (int i = 0; i < MAX_GOLD; i++)
    {
        float gx = (float)GetRandomValue(-280, -40);
        float gz = (float)GetRandomValue((int)BOAT_Z_MIN, (int)BOAT_Z_MAX);
        gold[i] = CreateGameObject("spps-voyage/models/gold_coin.glb", (Vector3){ gx, 1.5f, gz });
    }

    // ----------------------------------------------------------------
    // Crew & interior
    // ----------------------------------------------------------------
    CrewMember crew[MAX_CREW];
    InitCrewModel();          // loads roman_character.glb
    InitCrew(crew, MAX_CREW);

    InteriorState interior = { 0 };
    InitInterior(&interior);

    // ----------------------------------------------------------------
    // Game state
    // ----------------------------------------------------------------
    int   boatHealth     = 100;
    int   collectedBars  = 0;
    float waveTime       = 0.0f;
    bool  reachedMalta   = false;
    bool  moraleGameOver = false;

    int rockDistanceCounter    = 0;
    int goldDistanceCounter    = 0;
    int rockDistanceSeparation = 30;
    int goldDistanceSeparation = 30;

    // ----------------------------------------------------------------
    // Audio — two music tracks
    //
    // DECK MUSIC  → replace "spps-voyage/audio/btdb.wav" with your deck track
    // INTERIOR MUSIC → replace "spps-voyage/audio/btdb.wav" with your interior track
    // Both currently use the same file; swap the interior path when ready.
    // ----------------------------------------------------------------
    Music musicDeck     = LoadMusicStream("spps-voyage/audio/ocean_sounds.wav");
    Music musicInterior = LoadMusicStream("spps-voyage/audio/interior.wav");
    Sound crash         = LoadSound("spps-voyage/audio/crash.wav");
    Sound goldSound     = LoadSound("spps-voyage/audio/gold.wav");

    bool interiorMusicPlaying = false;

    if (IsMusicValid(musicDeck)) PlayMusicStream(musicDeck);

    // ----------------------------------------------------------------
    // Main game loop
    // ----------------------------------------------------------------
    while (!WindowShouldClose() && boatHealth > 0 && !reachedMalta && !moraleGameOver)
    {
        float dt = GetFrameTime();
        waveTime += dt;

        // ---- Music: swap tracks when Tab is pressed ----
        // (UpdateInterior handles the Tab toggle; we detect the change here)
        bool wasFullscreen = interior.fullscreen;

        // ---- Interior update (Tab, WASD, E) ----
        int nearestCrewIdx = -1;
        UpdateInterior(&interior, crew, MAX_CREW, dt, &nearestCrewIdx);

        // Swap music when view changes
        if (interior.fullscreen != wasFullscreen)
        {
            if (interior.fullscreen)
            {
                // Switched to interior
                if (IsMusicValid(musicDeck))     StopMusicStream(musicDeck);
                if (IsMusicValid(musicInterior)) PlayMusicStream(musicInterior);
                interiorMusicPlaying = true;
            }
            else
            {
                // Switched back to deck
                if (IsMusicValid(musicInterior)) StopMusicStream(musicInterior);
                if (IsMusicValid(musicDeck))     PlayMusicStream(musicDeck);
                interiorMusicPlaying = false;
            }
        }

        // Keep whichever track is active alive
        if (interiorMusicPlaying) { if (IsMusicValid(musicInterior)) UpdateMusicStream(musicInterior); }
        else                      { if (IsMusicValid(musicDeck))     UpdateMusicStream(musicDeck); }

        // ---- Crew update (always) ----
        UpdateCrew(crew, MAX_CREW, dt);

        float overallMorale = GetOverallMorale(crew, MAX_CREW);
        if (overallMorale <= MORALE_GAMEOVER_VAL) moraleGameOver = true;

        // ----------------------------------------------------------------
        // Boat / sailing logic — ALWAYS runs, even in interior view
        // ----------------------------------------------------------------
        boat.position.x -= 20.0f * dt;
        if (boat.position.x < FINISH_LINE_X) boat.position.x = FINISH_LINE_X;

        // Arrow keys always steer the boat regardless of which view is active
        if (IsKeyDown(KEY_LEFT))  boat.position.z += 3.0f * dt * 10.0f;
        if (IsKeyDown(KEY_RIGHT)) boat.position.z -= 3.0f * dt * 10.0f;

        if (boat.position.z > BOAT_Z_MAX) boat.position.z = BOAT_Z_MAX;
        if (boat.position.z < BOAT_Z_MIN) boat.position.z = BOAT_Z_MIN;

        boat.position.y = WaveHeight(boat.position.x, boat.position.z, waveTime) * 0.5f;

        // Camera always follows boat
        camera.position = (Vector3){ boat.position.x + 20.0f, boat.position.y + 10.0f, boat.position.z };
        camera.target   = (Vector3){ boat.position.x - 60.0f, boat.position.y +  2.0f, boat.position.z };

        if (boat.position.x <= FINISH_LINE_X) reachedMalta = true;

        UpdateRockSpawner(rocks, MAX_ROCKS, boat.position.x, &rockDistanceCounter, rockDistanceSeparation);
        UpdateGoldSpawner(gold,  MAX_GOLD,  boat.position.x, &goldDistanceCounter, goldDistanceSeparation);

        boatHealth -= CheckRockCollisions(&boat, rocks, MAX_ROCKS, crash);
        CheckGoldCollisions(&boat, gold, MAX_GOLD, goldSound, &collectedBars);

        // ----------------------------------------------------------------
        // Malta distance (clamped)
        // ----------------------------------------------------------------
        float distToMalta = boat.position.x - FINISH_LINE_X;
        if (distToMalta < 0) distToMalta = 0;

        // ----------------------------------------------------------------
        // Render interior to its texture (always live)
        // ----------------------------------------------------------------
        RenderInteriorToTexture(&interior, crew, MAX_CREW);

        // ----------------------------------------------------------------
        // Render ocean scene to mini texture (always live, used in
        // fullscreen interior mode as the deck mini-window)
        // ----------------------------------------------------------------
        BeginTextureMode(interior.oceanMiniTex);
            ClearBackground((Color){ 30, 90, 160, 255 });
            BeginMode3D(camera);
                DrawOcean(boat.position.x, waveTime);
                DrawModelEx(boat.model, boat.position,
                            (Vector3){ 0, 1, 0 }, 90.0f,
                            (Vector3){ 1, 1, 1 }, WHITE);
                for (int i = 0; i < MAX_ROCKS; i++)
                    if (rocks[i].active)
                        DrawModel(rocks[i].model, rocks[i].position, 1.0f, WHITE);
                for (int i = 0; i < MAX_GOLD; i++)
                    if (gold[i].active)
                        DrawModelEx(gold[i].model, gold[i].position,
                                    (Vector3){ 0, 1, 0 }, waveTime * 90.0f,
                                    (Vector3){ 3, 3, 3 }, GOLD);
                if (island.active)
                    DrawModelEx(island.model, island.position,
                                (Vector3){ 0, 1, 0 }, 0.0f, islandScale, WHITE);
            EndMode3D();
        EndTextureMode();

        // ----------------------------------------------------------------
        // Main framebuffer
        // ----------------------------------------------------------------
        BeginDrawing();

        if (interior.fullscreen)
        {
            // Interior fills the screen; DrawInteriorHUD draws the ocean
            // mini-window + full HUD on top.
            DrawInteriorHUD(&interior, crew, MAX_CREW,
                            boatHealth, overallMorale,
                            collectedBars, distToMalta);
        }
        else
        {
            // ---- Full ocean view ----
            ClearBackground((Color){ 30, 90, 160, 255 });
            DrawRectangle(0, HEIGHT / 2 - 40, WIDTH, 80, (Color){ 20, 70, 130, 255 });

            BeginMode3D(camera);
                DrawOcean(boat.position.x, waveTime);
                DrawModelEx(boat.model, boat.position,
                            (Vector3){ 0, 1, 0 }, 90.0f,
                            (Vector3){ 1, 1, 1 }, WHITE);
                for (int i = 0; i < MAX_ROCKS; i++)
                    if (rocks[i].active)
                        DrawModel(rocks[i].model, rocks[i].position, 1.0f, WHITE);
                for (int i = 0; i < MAX_GOLD; i++)
                    if (gold[i].active)
                        DrawModelEx(gold[i].model, gold[i].position,
                                    (Vector3){ 0, 1, 0 }, waveTime * 90.0f,
                                    (Vector3){ 3, 3, 3 }, GOLD);
                if (island.active)
                    DrawModelEx(island.model, island.position,
                                (Vector3){ 0, 1, 0 }, 0.0f, islandScale, WHITE);
            EndMode3D();

            // ---- HUD (left column) ----
            int hudX  = 20;
            int barW  = 200;
            int barH  = 24;
            int lineH = 36;
            int y     = 20;

            // 1. Health bar
            DrawBar(hudX, y, barW, barH,
                    (float)boatHealth, 100.0f,
                    (Color){200, 40, 40, 255},
                    TextFormat("HP: %d", boatHealth));
            y += lineH;

            // 2. Morale bar
            Color moraleColor = (overallMorale > 50.0f) ? (Color){60,200,80,255}
                              : (overallMorale > 25.0f) ? ORANGE : RED;
            DrawBar(hudX, y, barW, barH,
                    overallMorale, CREW_MORALE_MAX,
                    moraleColor,
                    TextFormat("Morale: %d%%", (int)overallMorale));
            y += lineH;

            // 3. Gold
            DrawText(TextFormat("Gold: %d", collectedBars), hudX, y, 26, GOLD);
            y += lineH;

            // 4. Malta distance
            DrawText(TextFormat("Malta: %.0f m", distToMalta), hudX, y, 22, WHITE);

            // ---- Interior mini-window (top-right) ----
            DrawInteriorHUD(&interior, crew, MAX_CREW,
                            boatHealth, overallMorale,
                            collectedBars, distToMalta);
        }

        EndDrawing();
    }

    // ----------------------------------------------------------------
    // End screen
    // ----------------------------------------------------------------
    const char* mainMsg   = "GAME OVER";
    const char* subMsg    = "You crashed before reaching Malta!";
    Color       mainColor = RED;

    if (reachedMalta)       { mainMsg = "YOU REACHED MALTA!";  subMsg = "Congratulations, Captain!"; mainColor = GREEN; }
    else if (moraleGameOver){ mainMsg = "MUTINY!";             subMsg = "Your crew lost all hope.";  mainColor = ORANGE; }

    // Stop music on end screen
    if (IsMusicValid(musicDeck))     StopMusicStream(musicDeck);
    if (IsMusicValid(musicInterior)) StopMusicStream(musicInterior);

    bool showingEndScreen = true;
    while (!WindowShouldClose() && showingEndScreen)
    {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE))
            showingEndScreen = false;

        BeginDrawing();
        ClearBackground(BLACK);

        DrawText(mainMsg,
                 WIDTH/2 - MeasureText(mainMsg, 54)/2, HEIGHT/2 - 80, 54, mainColor);
        DrawText(subMsg,
                 WIDTH/2 - MeasureText(subMsg, 28)/2,  HEIGHT/2 - 10, 28, WHITE);

        const char* goldStr = TextFormat("Gold collected: %d", collectedBars);
        DrawText(goldStr,
                 WIDTH/2 - MeasureText(goldStr, 36)/2, HEIGHT/2 + 50, 36, GOLD);

        DrawText("Press ENTER or SPACE to exit",
                 WIDTH/2 - MeasureText("Press ENTER or SPACE to exit", 20)/2,
                 HEIGHT/2 + 110, 20, GRAY);

        EndDrawing();
    }

    // ----------------------------------------------------------------
    // Cleanup
    // ----------------------------------------------------------------
    UnloadInterior(&interior);
    UnloadCrewModel();         // unloads roman_character.glb
    UnloadGameObject(&boat);
    UnloadGameObject(&island);
    for (int i = 0; i < MAX_ROCKS; i++) UnloadGameObject(&rocks[i]);
    for (int i = 0; i < MAX_GOLD;  i++) UnloadGameObject(&gold[i]);

    if (IsMusicValid(musicDeck))     UnloadMusicStream(musicDeck);
    if (IsMusicValid(musicInterior)) UnloadMusicStream(musicInterior);
    if (IsSoundValid(crash))         UnloadSound(crash);
    if (IsSoundValid(goldSound))     UnloadSound(goldSound);

    CloseAudioDevice();
    CloseWindow();
    return reachedMalta ? 0 : 1;
}
