#include "interior.h"

#define PAUL_SPEED      8.0f    // units per second (run speed)
#define PAUL_TURN_SPD  160.0f   // degrees per second (A/D turning)
#define PAUL_HEIGHT     1.6f    // eye level
#define PAUL_RADIUS     0.4f    // collision padding from walls

// ----------------------------------------------------------------
// Room geometry
// ----------------------------------------------------------------
static void DrawInteriorRoom(void)
{
    float hw = INTERIOR_ROOM_W * 0.5f;
    float hd = INTERIOR_ROOM_D * 0.5f;

    // Floor
    DrawPlane((Vector3){0, 0, 0},
              (Vector2){INTERIOR_ROOM_W, INTERIOR_ROOM_D},
              (Color){100, 70, 40, 255});

    // Ceiling
    DrawCube((Vector3){0, INTERIOR_ROOM_H, 0},
             INTERIOR_ROOM_W, 0.15f, INTERIOR_ROOM_D,
             (Color){60, 40, 20, 255});

    // Left wall
    DrawCube((Vector3){-hw, INTERIOR_ROOM_H * 0.5f, 0},
             0.15f, INTERIOR_ROOM_H, INTERIOR_ROOM_D,
             (Color){90, 60, 30, 255});

    // Right wall
    DrawCube((Vector3){ hw, INTERIOR_ROOM_H * 0.5f, 0},
             0.15f, INTERIOR_ROOM_H, INTERIOR_ROOM_D,
             (Color){90, 60, 30, 255});

    // Back wall
    DrawCube((Vector3){0, INTERIOR_ROOM_H * 0.5f, -hd},
             INTERIOR_ROOM_W, INTERIOR_ROOM_H, 0.15f,
             (Color){80, 55, 25, 255});

    // Front wall
    DrawCube((Vector3){0, INTERIOR_ROOM_H * 0.5f,  hd},
             INTERIOR_ROOM_W, INTERIOR_ROOM_H, 0.15f,
             (Color){80, 55, 25, 255});

    // Lanterns
    for (int i = -1; i <= 1; i += 2)
    {
        float lz = (float)i * hd * 0.5f;
        DrawCube((Vector3){-hw + 0.3f, INTERIOR_ROOM_H - 0.4f, lz}, 0.3f, 0.3f, 0.3f, YELLOW);
        DrawCube((Vector3){ hw - 0.3f, INTERIOR_ROOM_H - 0.4f, lz}, 0.3f, 0.3f, 0.3f, YELLOW);
    }

    // Benches
    DrawCube((Vector3){-hw + 0.6f, 0.3f,  hd * 0.3f}, 0.4f, 0.6f, 3.0f, (Color){80,50,20,255});
    DrawCube((Vector3){ hw - 0.6f, 0.3f, -hd * 0.3f}, 0.4f, 0.6f, 3.0f, (Color){80,50,20,255});
}

// ----------------------------------------------------------------
// Shared HUD bar helper (used inside DrawInteriorHUD for fullscreen)
// ----------------------------------------------------------------
static void DrawBarFS(int x, int y, int w, int h,
                      float value, float maxVal,
                      Color fillColor, const char* label)
{
    DrawRectangle(x - 2, y - 2, w + 4, h + 4, BLACK);
    DrawRectangle(x,     y,     w,     h,      DARKGRAY);
    int fillW = (int)(value / maxVal * (float)w);
    if (fillW > 0) DrawRectangle(x, y, fillW, h, fillColor);
    DrawText(label, x + 6, y + (h/2) - 9, 18, WHITE);
}

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

void InitInterior(InteriorState* s)
{
    s->paulPos    = (Vector3){ 0.0f, PAUL_HEIGHT, INTERIOR_ROOM_D * 0.4f };
    s->paulYaw    = 180.0f;
    s->fullscreen = false;

    s->camera.up         = (Vector3){ 0, 1, 0 };
    s->camera.fovy       = 70.0f;
    s->camera.projection = CAMERA_PERSPECTIVE;

    s->renderTex    = LoadRenderTexture(MINI_W, MINI_H);
    s->oceanMiniTex = LoadRenderTexture(MINI_W, MINI_H);
}

void UnloadInterior(InteriorState* s)
{
    UnloadRenderTexture(s->renderTex);
    UnloadRenderTexture(s->oceanMiniTex);
}

// ----------------------------------------------------------------
// Update — A/D turn, W/S move, Tab toggle, E interact
// ----------------------------------------------------------------
void UpdateInterior(InteriorState* s, CrewMember crew[], int crewCount,
                    float dt, int* nearestIdx)
{
    *nearestIdx = -1;

    if (IsKeyPressed(KEY_TAB)) s->fullscreen = !s->fullscreen;

    // Paul always moves with WASD regardless of which view is active
    // A/D — turn left/right
    if (IsKeyDown(KEY_A)) s->paulYaw -= PAUL_TURN_SPD * dt;
    if (IsKeyDown(KEY_D)) s->paulYaw += PAUL_TURN_SPD * dt;

    // W/S — forward/backward
    float rad = s->paulYaw * DEG2RAD;
    Vector3 fwd = { sinf(rad), 0, -cosf(rad) };

    if (IsKeyDown(KEY_W)) { s->paulPos.x += fwd.x * PAUL_SPEED * dt; s->paulPos.z += fwd.z * PAUL_SPEED * dt; }
    if (IsKeyDown(KEY_S)) { s->paulPos.x -= fwd.x * PAUL_SPEED * dt; s->paulPos.z -= fwd.z * PAUL_SPEED * dt; }

    // Clamp inside room
    float hw = INTERIOR_ROOM_W * 0.5f - PAUL_RADIUS;
    float hd = INTERIOR_ROOM_D * 0.5f - PAUL_RADIUS;
    if (s->paulPos.x >  hw) s->paulPos.x =  hw;
    if (s->paulPos.x < -hw) s->paulPos.x = -hw;
    if (s->paulPos.z >  hd) s->paulPos.z =  hd;
    if (s->paulPos.z < -hd) s->paulPos.z = -hd;

    // Find nearest crew member
    float minDist = 1e9f;
    int   nearest = -1;
    for (int i = 0; i < crewCount; i++)
    {
        float dx = crew[i].position.x - s->paulPos.x;
        float dz = crew[i].position.z - s->paulPos.z;
        float d  = sqrtf(dx*dx + dz*dz);
        if (d < minDist) { minDist = d; nearest = i; }
    }

    if (nearest != -1 && minDist <= CREW_INTERACT_DIST)
    {
        *nearestIdx = nearest; // for "Press E" prompt

        if (IsKeyPressed(KEY_E))
        {
            crew[nearest].morale += CREW_MORALE_BOOST;
            if (crew[nearest].morale > CREW_MORALE_MAX)
                crew[nearest].morale = CREW_MORALE_MAX;
            crew[nearest].boostedRecently = true;
            crew[nearest].boostTimer      = 0.5f;
        }
    }
}

// ----------------------------------------------------------------
// Render interior scene into renderTex
// ----------------------------------------------------------------
void RenderInteriorToTexture(InteriorState* s, CrewMember crew[], int crewCount)
{
    float rad = s->paulYaw * DEG2RAD;
    Vector3 lookDir = { sinf(rad), 0, -cosf(rad) };
    s->camera.position = s->paulPos;
    s->camera.target   = Vector3Add(s->paulPos, lookDir);

    BeginTextureMode(s->renderTex);
        ClearBackground((Color){40, 25, 10, 255});
        BeginMode3D(s->camera);
            DrawInteriorRoom();
            DrawCrew(crew, crewCount);
        EndMode3D();
    EndTextureMode();
}

// Called from main to blit the current ocean frame into the ocean mini texture.
// main passes its own BeginTextureMode block — we expose the texture directly.
// Instead, main calls BeginTextureMode(interior.oceanMiniTex) itself.
// (Nothing needed here — oceanMiniTex is public via the struct.)

// ----------------------------------------------------------------
// Draw HUD — mini window on deck view, or fullscreen interior with
// ocean mini-window + full HUD overlay
// ----------------------------------------------------------------
void DrawInteriorHUD(const InteriorState* s,
                     const CrewMember crew[], int crewCount,
                     int boatHealth, float overallMorale,
                     int collectedBars, float distToMalta)
{
    // Flip rect (OpenGL textures are upside-down)
    Rectangle srcFull = { 0, 0, (float)MINI_W, -(float)MINI_H };

    if (s->fullscreen)
    {
        // ---- Full-screen interior ----
        Rectangle destFull = { 0, 0, (float)WIDTH, (float)HEIGHT };
        DrawTexturePro(s->renderTex.texture, srcFull, destFull, (Vector2){0,0}, 0.0f, WHITE);

        // ---- Ocean mini-window (top-right, same spot as interior mini-window on deck) ----
        int mx = WIDTH  - MINI_W - 10;
        int my = 10;
        DrawRectangle(mx - 2, my - 2, MINI_W + 4, MINI_H + 4, BLACK);
        Rectangle destMini = { (float)mx, (float)my, (float)MINI_W, (float)MINI_H };
        DrawTexturePro(s->oceanMiniTex.texture, srcFull, destMini, (Vector2){0,0}, 0.0f, WHITE);
        DrawText("DECK  [TAB]", mx + 4, my + 4, 14, (Color){255,255,255,200});

        // ---- HUD (left column — same layout as deck view) ----
        int hudX  = 20;
        int barW  = 200;
        int barH  = 24;
        int lineH = 36;
        int y     = 20;

        // 1. Health bar
        DrawBarFS(hudX, y, barW, barH,
                  (float)boatHealth, 100.0f,
                  (Color){200, 40, 40, 255},
                  TextFormat("HP: %d", boatHealth));
        y += lineH;

        // 2. Morale bar
        Color moraleColor = (overallMorale > 50.0f) ? (Color){60,200,80,255}
                          : (overallMorale > 25.0f) ? ORANGE : RED;
        DrawBarFS(hudX, y, barW, barH,
                  overallMorale, CREW_MORALE_MAX,
                  moraleColor,
                  TextFormat("Morale: %d%%", (int)overallMorale));
        y += lineH;

        // 3. Gold
        DrawText(TextFormat("Gold: %d", collectedBars), hudX, y, 26, GOLD);
        y += lineH;

        // 4. Malta distance
        DrawText(TextFormat("Malta: %.0f m", distToMalta), hudX, y, 22, WHITE);

        // ---- Controls hint bar at top ----
        DrawRectangle(0, 0, WIDTH, 34, (Color){0,0,0,150});
        DrawText("INTERIOR  |  W/S move   A/D turn   E boost crew   ARROWS steer boat   TAB return to deck",
                 10, 8, 16, LIGHTGRAY);

        // ---- "Press E" prompt ----
        float minDist = 1e9f;
        for (int i = 0; i < crewCount; i++)
        {
            float dx = crew[i].position.x - s->paulPos.x;
            float dz = crew[i].position.z - s->paulPos.z;
            float d  = sqrtf(dx*dx + dz*dz);
            if (d < minDist) minDist = d;
        }
        if (minDist <= CREW_INTERACT_DIST)
        {
            const char* prompt = "Press E to boost morale";
            int tw = MeasureText(prompt, 22);
            DrawRectangle(WIDTH/2 - tw/2 - 10, HEIGHT - 70, tw + 20, 36, (Color){0,0,0,180});
            DrawText(prompt, WIDTH/2 - tw/2, HEIGHT - 62, 22, YELLOW);
        }
    }
    else
    {
        // ---- Mini interior window (top-right, deck view) ----
        int mx = WIDTH  - MINI_W - 10;
        int my = 10;
        DrawRectangle(mx - 2, my - 2, MINI_W + 4, MINI_H + 4, BLACK);
        Rectangle destMini = { (float)mx, (float)my, (float)MINI_W, (float)MINI_H };
        DrawTexturePro(s->renderTex.texture, srcFull, destMini, (Vector2){0,0}, 0.0f, WHITE);
        DrawText("INTERIOR  [TAB]", mx + 4, my + 4, 14, (Color){255,255,255,200});
    }
}