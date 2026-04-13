#ifndef INTERIOR_H
#define INTERIOR_H

#include "game.h"
#include "crew.h"

typedef struct {
    // Paul's first-person position & yaw inside the interior
    Vector3 paulPos;
    float   paulYaw;          // degrees, horizontal look

    Camera3D camera;
    RenderTexture2D renderTex;      // interior scene drawn here
    RenderTexture2D oceanMiniTex;   // ocean scene drawn here (for the mini-window inside fullscreen)

    bool fullscreen;          // Tab toggles this
} InteriorState;

// Call once at startup
void InitInterior(InteriorState* s);

// Call once on shutdown
void UnloadInterior(InteriorState* s);

// Handle Paul movement (WASD), Tab toggle, E interaction.
// nearestIdx is set to the crew index in interact range (-1 if none).
// E pressed on a nearby crew member boosts their morale.
void UpdateInterior(InteriorState* s, CrewMember crew[], int crewCount,
                    float dt, int* nearestIdx);

// Render the interior scene into s->renderTex.
// Call BEFORE BeginDrawing().
void RenderInteriorToTexture(InteriorState* s, CrewMember crew[], int crewCount);

// Draw the mini interior window (always top-right) OR the fullscreen overlay.
// In fullscreen mode also draws the ocean mini-window and full HUD.
// Call INSIDE BeginDrawing() / EndDrawing().
// Pass all HUD values so they can be redrawn in fullscreen mode.
void DrawInteriorHUD(const InteriorState* s,
                     const CrewMember crew[], int crewCount,
                     int boatHealth, float overallMorale,
                     int collectedBars, float distToMalta);

#endif // INTERIOR_H
