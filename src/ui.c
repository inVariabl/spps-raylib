#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

void DrawInventory(Player *player) {
    if (!player->showInventory) return;

    int questStartY = GetScreenHeight() - 450;
    int questHeight = 90;
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
            Color itemCol = itemDatabase[player->inventory[i].itemId].color;
            DrawRectangle(slotX + 5, slotY + 5, 30, 25, itemCol);
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

    int starsX = GetScreenWidth() - 170;
    int starsY = 12;
    DrawRectangle(starsX - 10, starsY - 4, 150, 28, Fade(BLACK, 0.55f));
    for (int i = 0; i < 5; i++) {
        Color starColor = (i < player->wantedStars) ? GOLD : Fade(LIGHTGRAY, 0.4f);
        DrawText("*", starsX + i * 24, starsY, 24, starColor);
    }

    // Location Display
    DrawRectangle(10, 40, 200, 50, Fade(DARKGRAY, 0.7f));
    DrawText("COORDINATES", 15, 45, 12, GOLD);
    char buf[64];
    sprintf(buf, "X: %d, Z: %d", player->position.x, player->position.z);
    DrawText(buf, 15, 60, 20, WHITE);

    DrawRectangle(10, 95, 200, 24, Fade(BLACK, 0.6f));
    DrawText(IsPlayerSeenByPharisee(world) ? "WATCHED: PHARISEE" : "WATCHED: CLEAR",
             18, 101, 14, IsPlayerSeenByPharisee(world) ? RED : GREEN);

    // --- NAVIGATION COMPASS ---
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
        DrawRectangle(10, 125, 300, 28, Fade(BLACK, 0.6f));
        DrawText(TextFormat("Objective: Travel to %s", targetName), 18, 131, 16, YELLOW);

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
        DrawRectangle(10, 125, 300, 28, Fade(BLACK, 0.6f));
        DrawText("Objective: Follow the road to Rome", 18, 131, 16, YELLOW);
    }

    if (world->state.hasJulius) {
        Vector3Int j = world->state.juliusPos;
        if (abs(player->position.x - j.x) <= 5 && abs(player->position.z - j.z) <= 5) {
            DrawRectangle(10, 155, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Julius: You may visit friends in Sidon.", 18, 159, 14, SKYBLUE);
        }
    }
    if (world->state.hasSnake) {
        Vector3Int s = world->state.snakePos;
        if (abs(player->position.x - s.x) <= 6 && abs(player->position.z - s.z) <= 6) {
            DrawRectangle(10, 185, 360, 24, Fade(BLACK, 0.6f));
            DrawText("A viper strikes, but Paul is unharmed.", 18, 189, 14, ORANGE);
        }
    }
    if (!player->gameComplete && world->state.hasHouseArrest) {
        Vector3Int h = world->state.houseArrestPos;
        if (abs(player->position.x - h.x) <= 4 && abs(player->position.z - h.z) <= 4) {
            DrawRectangle(10, 215, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Press H to enter house arrest", 18, 219, 14, GOLD);
        }
    }
    if (player->gameComplete) {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.6f));
        DrawText("Arrived in Rome - House Arrest", w/2 - 170, h/2 - 10, 20, GOLD);
    }

    if (world->state.nearbyPreachNpcIndex != -1) {
        NPC *npc = &world->state.npcs[world->state.nearbyPreachNpcIndex];
        int promptW = 360;
        int promptX = GetScreenWidth() / 2 - promptW / 2;
        int promptY = GetScreenHeight() - 115;
        DrawRectangle(promptX, promptY, promptW, 52, Fade(BLACK, 0.7f));
        DrawRectangleLines(promptX, promptY, promptW, 52, GOLD);
        DrawText(TextFormat("Hold E to preach the Gospel to %s", npc->name),
                 promptX + 14, promptY + 8, 18, GOLD);

        Color statusColor = world->state.nearbyPreachNpcSeesPlayer ? GREEN : ORANGE;
        const char *statusText = world->state.nearbyPreachNpcSeesPlayer ?
            "They are listening" : "Wait until they are looking at you";
        DrawText(statusText, promptX + 14, promptY + 28, 14, statusColor);

        DrawRectangle(promptX + 14, promptY + 44, promptW - 28, 6, DARKGRAY);
        int holdWidth = (int)((float)(promptW - 28) * (player->preachHoldTimer / 3.0f));
        if (holdWidth > promptW - 28) holdWidth = promptW - 28;
        DrawRectangle(promptX + 14, promptY + 44, holdWidth, 6, LIME);
    }

    if (player->preachSuccessTimer > 0.0f) {
        int msgW = 360;
        int msgX = GetScreenWidth() / 2 - msgW / 2;
        int msgY = 40;
        DrawRectangle(msgX, msgY, msgW, 36, Fade(DARKGREEN, 0.85f));
        DrawRectangleLines(msgX, msgY, msgW, 36, GOLD);
        DrawText("The Gospel was received. Spirit increased.", msgX + 16, msgY + 10, 18, WHITE);
    }

    // QUEST LOG
    int questStartX = GetScreenWidth() - 220;
    int questStartY = GetScreenHeight() - 450;
    DrawRectangle(questStartX, questStartY, 200, 90, Fade(BLUE, 0.4f));
    DrawRectangleLines(questStartX, questStartY, 200, 90, SKYBLUE);
    DrawText("QUEST LOG", questStartX + 10, questStartY + 10, 15, GOLD);
    
    if (player->activeQuestId != 0) {
        int qId = player->activeQuestId;
        DrawText(questDatabase[qId].title, questStartX + 10, questStartY + 35, 12, WHITE);
        // Truncate description for log
        DrawText(TextSubtext(questDatabase[qId].description, 0, 30), questStartX + 10, questStartY + 55, 10, GRAY);
    } else {
        DrawText("No active mission.", questStartX + 10, questStartY + 40, 12, GRAY);
    }
}
