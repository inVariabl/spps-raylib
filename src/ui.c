#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

static bool PlayerHasItem(const Player *player, int itemId) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].itemId == itemId) {
            return true;
        }
    }
    return false;
}

static void GetQuestLogText(const Player *player, const World *world,
                            const char **title, const char **line1, const char **line2) {
    *title = "Current Objective";
    *line1 = "Keep moving forward.";
    *line2 = "";

    if (world->state.worldId == WORLD_JUDEA) {
        *title = "Jerusalem";
        if (!PlayerHasItem(player, 5)) {
            *line1 = "Collect the letters nearby.";
            *line2 = "Talk to people in the city.";
        } else if (player->questStates[1] == QUEST_NOT_STARTED) {
            *line1 = "Talk to people in Jerusalem.";
            *line2 = "Look for your next assignment.";
        } else if (player->questStates[1] == QUEST_ACTIVE) {
            *line1 = "Carry the scroll onward.";
            *line2 = "Head toward the coast to depart.";
        } else {
            *line1 = "Make your way to the ship.";
            *line2 = "Travel when you are ready.";
        }
        return;
    }

    if (world->state.worldId == WORLD_MALTA) {
        *title = "Malta";
        if (player->questStates[3] == QUEST_NOT_STARTED) {
            *line1 = "Click the fire to light it.";
            *line2 = "Stay with the islanders.";
        } else if (player->questStates[3] == QUEST_ACTIVE) {
            *line1 = "The viper has struck.";
            *line2 = "Wait and see what happens.";
        } else {
            *line1 = "Return to the ship at port.";
            *line2 = "Sail onward when ready.";
        }
        return;
    }

    if (world->state.worldId == WORLD_PUTEOLI) {
        *title = "Road to Rome";
        if (!player->gameComplete) {
            *line1 = "Travel north toward Rome.";
            *line2 = "Reach the house at the end.";
        } else {
            *line1 = "Press H near the house.";
            *line2 = "Enter house arrest.";
        }
    }
}

static void DrawItemIcon(Rectangle bounds, int itemId) {
    Color itemCol = itemDatabase[itemId].color;
    int x = (int)bounds.x;
    int y = (int)bounds.y;
    int w = (int)bounds.width;
    int h = (int)bounds.height;

    switch (itemId) {
        case 4: // Tent canvas as a folded envelope instead of a plain cloth block
        {
            Color paper = (Color){232, 214, 174, 255};
            DrawRectangle(x + 3, y + 5, w - 6, h - 10, paper);
            DrawRectangleLines(x + 3, y + 5, w - 6, h - 10, DARKBROWN);
            DrawLine(x + 3, y + 5, x + w/2, y + h/2, DARKBROWN);
            DrawLine(x + w - 3, y + 5, x + w/2, y + h/2, DARKBROWN);
            DrawLine(x + 3, y + h - 5, x + w/2, y + h/2 + 1, DARKBROWN);
            DrawLine(x + w - 3, y + h - 5, x + w/2, y + h/2 + 1, DARKBROWN);
        } break;
        case 5: // Letter quest item
        {
            Color paper = (Color){244, 232, 196, 255};
            DrawRectangle(x + 4, y + 6, w - 8, h - 12, paper);
            DrawRectangleLines(x + 4, y + 6, w - 8, h - 12, DARKBROWN);
            DrawLine(x + 4, y + 6, x + w/2, y + h/2, DARKBROWN);
            DrawLine(x + w - 4, y + 6, x + w/2, y + h/2, DARKBROWN);
            DrawCircle(x + w - 8, y + h - 9, 3, RED);
        } break;
        default:
            DrawRectangle(x + 5, y + 5, w - 10, h - 10, itemCol);
            DrawRectangleLines(x + 5, y + 5, w - 10, h - 10, Fade(RAYWHITE, 0.25f));
            break;
    }
}

void DrawInventory(Player *player) {
    if (!player->showInventory) return;

    int questStartY = GetScreenHeight() - 450;
    int questHeight = 110;
    int invStartX = GetScreenWidth() - 220;
    int invStartY = questStartY + questHeight + 10;

    // Inventory Panel
    DrawRectangle(invStartX, invStartY, 200, 330, Fade(BROWN, 0.9f));
    DrawRectangleLines(invStartX, invStartY, 200, 330, GOLD);
    DrawText("INVENTORY", invStartX + 50, invStartY + 10, 18, GOLD);

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int slotX = invStartX + 10 + (i % 4) * 46;
        int slotY = invStartY + 40 + (i / 4) * 40;

        DrawRectangle(slotX, slotY, 40, 35, BLACK); // Slot background

        if (player->inventory[i].itemId != 0) {
            DrawItemIcon((Rectangle){(float)slotX + 3, (float)slotY + 3, 34, 29},
                         player->inventory[i].itemId);
        }
    }

    // Craft Button
    Rectangle craftBtn = {(float)invStartX + 10, (float)invStartY + 290, 180, 30};
    bool hovered = CheckCollisionPointRec(GetMousePosition(), craftBtn);
    DrawRectangleRec(craftBtn, hovered ? GOLD : BLACK);
    DrawRectangleLinesEx(craftBtn, 2, GOLD);
    DrawText("CRAFT TENT", invStartX + 45, invStartY + 297, 15, hovered ? BLACK : GOLD);

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        TryCraftTent(player);
    }
}

void DrawSkills(Player *player) {
    // Basic skills UI top-left
    DrawRectangle(10, 100, 150, 80, Fade(DARKGRAY, 0.7f));
    DrawText("SKILLS", 15, 105, 15, WHITE);
    
    char buf[64];
    sprintf(buf, "Oratory: %d", player->skills[SKILL_ORATORY].level);
    DrawText(buf, 15, 125, 12, GOLD);
    sprintf(buf, "Tentmaking: %d", player->skills[SKILL_TENTMAKING].level);
    DrawText(buf, 15, 140, 12, GOLD);
    sprintf(buf, "Endurance: %d", player->skills[SKILL_ENDURANCE].level);
    DrawText(buf, 15, 155, 12, GOLD);
}

#include "world.h"

void DrawHUD(Player *player, World *world, bool isFirstPerson) {
    if (isFirstPerson) {
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;
        DrawLine(centerX - 10, centerY, centerX + 10, centerY, GREEN);
        DrawLine(centerX, centerY - 10, centerX, centerY + 10, GREEN);
    }

    // Health/Spirit bar
    DrawRectangle(10, 10, 200, 25, BLACK);
    float hpPct = (float)player->spirit / (float)player->maxSpirit;
    DrawRectangle(12, 12, (int)(196 * hpPct), 21, RED);
    DrawText("SPIRIT", 80, 15, 12, WHITE);

    // Location Display
    DrawRectangle(10, 40, 200, 50, Fade(DARKGRAY, 0.7f));
    DrawText("COORDINATES", 15, 45, 12, GOLD);
    char buf[64];
    sprintf(buf, "X: %d, Z: %d", player->position.x, player->position.z);
    DrawText(buf, 15, 60, 20, WHITE);

    // ---NAVIGATION COMPASS ---
    if (player->showMap) {
        int compassW = 400;
        int compassX = GetScreenWidth()/2 - compassW/2;
        int compassY = 30;
        DrawRectangle(compassX, compassY, compassW, 30, Fade(BLACK, 0.6f));
        DrawRectangleLines(compassX, compassY, compassW, 30, GOLD);

        // North/South/East/West markers
        DrawText("W", compassX + 5, compassY + 8, 15, WHITE);
        DrawText("E", compassX + compassW - 15, compassY + 8, 15, WHITE);

        // Nearby Cities on compass
        for (int i = 0; i < sizeof(worldMap)/sizeof(City); i++) {
            if (worldMap[i].coords.x < world->state.minX || worldMap[i].coords.x > world->state.maxX ||
                worldMap[i].coords.z < world->state.minZ || worldMap[i].coords.z > world->state.maxZ) {
                continue;
            }
            // Calculate relative direction
            float dx = (float)worldMap[i].coords.x - player->lerpPosition.x;
            float dz = (float)worldMap[i].coords.z - player->lerpPosition.z;
            float dist = sqrtf(dx*dx + dz*dz);
            
            if (dist < 600.0f) {
                // Map angle to X position on compass bar (-45 to 45 degrees visible)
                // For a simple horizontal bar, we use the relative X/Z
                float relativeX = dx / 600.0f; // -1 to 1
                int markerX = compassX + compassW/2 + (int)(relativeX * (compassW/2));
                
                if (markerX > compassX + 5 && markerX < compassX + compassW - 15) {
                    DrawRectangle(markerX - 2, compassY + 5, 4, 20, GOLD);
                    if (dist < 100.0f) DrawText(worldMap[i].name, markerX - 20, compassY - 20, 12, GOLD);
                }
            }
        }

        // Overview Map (Top-Right)
        int mapW = 150;
        int mapH = 150;
        int mapX = GetScreenWidth() - mapW - 10;
        int mapY = 10;
        DrawRectangle(mapX, mapY, mapW, mapH, Fade(BLACK, 0.8f));
        DrawRectangleLines(mapX, mapY, mapW, mapH, GOLD);

        // Draw cities on map
        for (int i = 0; i < sizeof(worldMap)/sizeof(City); i++) {
            if (worldMap[i].coords.x < world->state.minX || worldMap[i].coords.x > world->state.maxX ||
                worldMap[i].coords.z < world->state.minZ || worldMap[i].coords.z > world->state.maxZ) {
                continue;
            }
            int cityX = mapX + mapW/2 + worldMap[i].coords.x / 5;
            int cityZ = mapY + mapH/2 + worldMap[i].coords.z / 5;
            if (cityX > mapX && cityX < mapX + mapW && cityZ > mapY && cityZ < mapY + mapH) {
                DrawCircle(cityX, cityZ, 2, RED);
            }
        }
        // Draw player
        int px = mapX + mapW/2 + (int)player->lerpPosition.x / 5;
        int pz = mapY + mapH/2 + (int)player->lerpPosition.z / 5;
        if (px > mapX && px < mapX + mapW && pz > mapY && pz < mapY + mapH) {
            DrawCircle(px, pz, 3, GREEN);
        }
    }

    // Objective + Port travel prompt
    int portIdx = GetPortAt(world, player->position);
    if (world->state.nextWorldId != WORLD_NONE) {
        const char *targetName = world->state.nextWorldName;
        DrawRectangle(10, 90, 300, 28, Fade(BLACK, 0.6f));
        DrawText(TextFormat("Objective: Travel to %s", targetName), 18, 96, 16, YELLOW);

        if (portIdx != -1) {
            int msgX = GetScreenWidth() / 2 - 140;
            int msgY = GetScreenHeight() - 80;
            DrawRectangle(msgX - 10, msgY - 8, 320, 26, Fade(BLACK, 0.6f));
            const char *fromName = world->state.ports[portIdx].name;
            const char *toName = world->state.nextWorldName;
            DrawText(TextFormat("Press T to travel from %s to %s", fromName, toName),
                     msgX, msgY, 16, SKYBLUE);
        }
    } else {
        DrawRectangle(10, 90, 300, 28, Fade(BLACK, 0.6f));
        DrawText("Objective: Follow the road to Rome", 18, 96, 16, YELLOW);
    }

    if (world->state.hasJulius) {
        Vector3Int j = world->state.juliusPos;
        if (abs(player->position.x - j.x) <= 5 && abs(player->position.z - j.z) <= 5) {
            DrawRectangle(10, 120, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Julius: You may visit friends in Sidon.", 18, 124, 14, SKYBLUE);
        }
    }
    if (!player->gameComplete && world->state.hasHouseArrest) {
        Vector3Int h = world->state.houseArrestPos;
        if (abs(player->position.x - h.x) <= 4 && abs(player->position.z - h.z) <= 4) {
            DrawRectangle(10, 180, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Press H to enter house arrest", 18, 184, 14, GOLD);
        }
    }
    if (player->gameComplete) {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.6f));
        DrawText("Arrived in Rome - House Arrest", w/2 - 170, h/2 - 10, 20, GOLD);
    }

    // QUEST LOG
    int questStartX = GetScreenWidth() - 220;
    int questStartY = GetScreenHeight() - 450;
    DrawRectangle(questStartX, questStartY, 200, 110, Fade(BLUE, 0.4f));
    DrawRectangleLines(questStartX, questStartY, 200, 110, SKYBLUE);
    DrawText("QUEST LOG", questStartX + 10, questStartY + 10, 15, GOLD);

    const char *questTitle;
    const char *line1;
    const char *line2;
    GetQuestLogText(player, world, &questTitle, &line1, &line2);

    DrawText(questTitle, questStartX + 10, questStartY + 35, 12, WHITE);
    DrawText(line1, questStartX + 10, questStartY + 55, 10, WHITE);
    if (line2[0] != '\0') {
        DrawText(line2, questStartX + 10, questStartY + 70, 10, WHITE);
    }
}

static float DrawSimpleSlider(Rectangle rect, const char *text, float value, float min, float max) {
    // Background track
    DrawRectangleRec(rect, Fade(BLACK, 0.5f));
    DrawRectangleLinesEx(rect, 1, WHITE);
    
    // Calculate knob position
    float range = max - min;
    float normalized = (value - min) / range;
    float knobX = rect.x + (normalized * (rect.width - 16));
    Rectangle knob = { knobX, rect.y + 2, 16, rect.height - 4 };
    
    // Handle Input
    if (CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float mouseX = GetMousePosition().x;
        float newNormalized = (mouseX - rect.x - 8) / (rect.width - 16);
        if (newNormalized < 0.0f) newNormalized = 0.0f;
        if (newNormalized > 1.0f) newNormalized = 1.0f;
        value = min + (newNormalized * range);
    }
    
    // Draw Knob
    DrawRectangleRec(knob, RED);
    
    // Draw Label
    DrawText(TextFormat("%s: %.2f", text, value), rect.x + rect.width + 10, rect.y + 2, 10, WHITE);
    
    return value;
}

void DrawShaderDebugUI(ShaderSettings *settings) {
    if (!settings->showDebugUI) return;

    int w = 350;
    int h = 400;
    int x = GetScreenWidth() - w - 20;
    int y = 50;
    
    DrawRectangle(x, y, w, h, Fade(DARKBLUE, 0.9f));
    DrawRectangleLines(x, y, w, h, WHITE);
    DrawText("SHADER SETTINGS", x + 10, y + 10, 20, WHITE);
    
    int startY = y + 50;
    int spacing = 30;
    
    // Light Direction
    settings->lightDir.x = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY, 200, 20}, "Light X", settings->lightDir.x, -1.0f, 1.0f);
    settings->lightDir.y = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing, 200, 20}, "Light Y", settings->lightDir.y, -1.0f, 1.0f);
    settings->lightDir.z = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing*2, 200, 20}, "Light Z", settings->lightDir.z, -1.0f, 1.0f);
    
    // Ambient
    settings->ambient = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing*3, 200, 20}, "Ambient", settings->ambient, 0.0f, 1.0f);
    
    // Shadow Bias
    settings->shadowBias = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing*4, 200, 20}, "Bias", settings->shadowBias, 0.0001f, 0.01f);
    
    // Color (R, G, B)
    float r = (float)settings->lightColor.r / 255.0f;
    float g = (float)settings->lightColor.g / 255.0f;
    float b = (float)settings->lightColor.b / 255.0f;
    
    r = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing*5, 200, 20}, "Color R", r, 0.0f, 1.0f);
    g = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing*6, 200, 20}, "Color G", g, 0.0f, 1.0f);
    b = DrawSimpleSlider((Rectangle){(float)x + 20, (float)startY + spacing*7, 200, 20}, "Color B", b, 0.0f, 1.0f);
    
    settings->lightColor = (Color){ (unsigned char)(r*255), (unsigned char)(g*255), (unsigned char)(b*255), 255 };

    // Save Button
    Rectangle saveBtn = { (float)x + 20, (float)startY + spacing*9, 100, 30 };
    bool hovered = CheckCollisionPointRec(GetMousePosition(), saveBtn);
    DrawRectangleRec(saveBtn, hovered ? GREEN : DARKGRAY);
    DrawText("SAVE", saveBtn.x + 30, saveBtn.y + 8, 10, WHITE);
    
    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        FILE *f = fopen("shader_settings.txt", "w");
        if (f) {
            fprintf(f, "%f %f %f\n", settings->lightDir.x, settings->lightDir.y, settings->lightDir.z);
            fprintf(f, "%d %d %d\n", settings->lightColor.r, settings->lightColor.g, settings->lightColor.b);
            fprintf(f, "%f\n", settings->ambient);
            fprintf(f, "%f\n", settings->shadowBias);
            fclose(f);
        }
    }
}
