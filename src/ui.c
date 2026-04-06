#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct {
    const char *line;
    const char *reference;
    const char *answers[4];
    int correct;
} GuardQuestion;

static const GuardQuestion guardQuestions[] = {
    {
        "Roman Guard: \"You claim you are Paul of Tarsus. Tell me this... who spoke to you on the road to Damascus?\"",
        "Acts 22:6-8",
        {
            "1. It was an angel of the temple.",
            "2. Jesus of Nazareth spoke to me from heaven.",
            "3. A Roman officer stopped me on the road.",
            "4. I do not remember who it was."
        },
        1
    },
    {
        "Roman Guard: \"Then explain this... what were you doing before that moment on the Damascus road?\"",
        "Acts 22:4-5",
        {
            "1. I was preaching in the synagogues.",
            "2. I was traveling as a merchant.",
            "3. I was persecuting the followers of Jesus.",
            "4. I was serving the Roman government."
        },
        2
    },
    {
        "Roman Guard: \"And when you were blinded... who restored your sight?\"",
        "Acts 22:12-13",
        {
            "1. Peter healed me in Jerusalem.",
            "2. A Roman physician restored my sight.",
            "3. A man named Ananias prayed for me.",
            "4. I recovered my sight on my own."
        },
        2
    }
};

static int GetNearbyGuardIndex(World *world, Player *player) {
    for (int i = 0; i < 20; i++) {
        NPC *npc = &world->state.npcs[i];
        if (!npc->active) continue;
        if (!TextIsEqual(npc->name, "Guard")) continue;

        float distance = Vector2Distance(
            (Vector2){npc->lerpPosition.x, npc->lerpPosition.z},
            (Vector2){player->lerpPosition.x, player->lerpPosition.z}
        );

        if (distance <= 2.25f) return i;
    }

    return -1;
}

static int DrawWrappedTextBlock(const char *text, int x, int y, int fontSize, int maxWidth, int lineSpacing, Color color) {
    char line[1024] = {0};
    char word[256] = {0};
    int lineCount = 0;
    int wordLen = 0;
    int lineLen = 0;

    for (int i = 0;; i++) {
        char c = text[i];
        bool flushWord = (c == ' ' || c == '\n' || c == '\0');

        if (!flushWord) {
            if (wordLen < (int)sizeof(word) - 1) {
                word[wordLen++] = c;
            }
            continue;
        }

        word[wordLen] = '\0';

        if (wordLen > 0) {
            char testLine[1024] = {0};

            if (lineLen == 0) {
                snprintf(testLine, sizeof(testLine), "%s", word);
            } else {
                snprintf(testLine, sizeof(testLine), "%s %s", line, word);
            }

            if (MeasureText(testLine, fontSize) > maxWidth && lineLen > 0) {
                DrawText(line, x, y + lineCount * (fontSize + lineSpacing), fontSize, color);
                lineCount++;
                snprintf(line, sizeof(line), "%s", word);
                lineLen = (int)strlen(line);
            } else {
                if (lineLen == 0) {
                    snprintf(line, sizeof(line), "%s", word);
                } else {
                    strncat(line, " ", sizeof(line) - strlen(line) - 1);
                    strncat(line, word, sizeof(line) - strlen(line) - 1);
                }
                lineLen = (int)strlen(line);
            }
        }

        wordLen = 0;

        if (c == '\n') {
            if (lineLen > 0) {
                DrawText(line, x, y + lineCount * (fontSize + lineSpacing), fontSize, color);
                lineCount++;
                line[0] = '\0';
                lineLen = 0;
            } else {
                lineCount++;
            }
        }

        if (c == '\0') break;
    }

    if (lineLen > 0) {
        DrawText(line, x, y + lineCount * (fontSize + lineSpacing), fontSize, color);
        lineCount++;
    }

    return lineCount;
}

void UpdateGuardDialogue(Player *player, World *world) {
    int guardIdx = GetNearbyGuardIndex(world, player);
    int questionCount = (int)(sizeof(guardQuestions) / sizeof(guardQuestions[0]));

    if (!player->guardDialogueActive) {
        if (guardIdx != -1 && IsKeyPressed(KEY_E)) {
            player->guardDialogueActive = true;
            player->guardDialogueStep = 0;
            player->guardDialogueLastResult = -1;
        }
        return;
    }

    if (player->guardClearedForShip) {
        if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            player->guardDialogueActive = false;
        }
        return;
    }

    int choice = -1;
    if (IsKeyPressed(KEY_ONE)) choice = 0;
    else if (IsKeyPressed(KEY_TWO)) choice = 1;
    else if (IsKeyPressed(KEY_THREE)) choice = 2;
    else if (IsKeyPressed(KEY_FOUR)) choice = 3;

    if (choice == -1) return;

    if (choice == guardQuestions[player->guardDialogueStep].correct) {
        player->guardDialogueLastResult = 1;
        player->guardDialogueStep++;

        if (player->guardDialogueStep >= questionCount) {
            player->guardClearedForShip = true;
        }
    } else {
        player->guardDialogueLastResult = 0;
    }
}

void DrawInventory(Player *player) {
    if (!player->showInventory) return;

    int questStartY = GetScreenHeight() - 450;
    int questHeight = 90;
    int invStartX = GetScreenWidth() - 220;
    int invStartY = questStartY + questHeight + 10;

    DrawRectangle(invStartX, invStartY, 200, 330, Fade(BROWN, 0.9f));
    DrawRectangleLines(invStartX, invStartY, 200, 330, GOLD);
    DrawText("INVENTORY", invStartX + 50, invStartY + 10, 18, GOLD);

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int slotX = invStartX + 10 + (i % 4) * 46;
        int slotY = invStartY + 40 + (i / 4) * 40;

        DrawRectangle(slotX, slotY, 40, 35, BLACK);

        if (player->inventory[i].itemId != 0) {
            Color itemCol = itemDatabase[player->inventory[i].itemId].color;
            DrawRectangle(slotX + 5, slotY + 5, 30, 25, itemCol);
        }
    }

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
    DrawRectangle(10, 155, 150, 60, Fade(DARKGRAY, 0.7f));
    DrawText("SKILLS", 15, 160, 15, WHITE);

    char buf[64];
    sprintf(buf, "Oratory: %d", player->skills[SKILL_ORATORY].level);
    DrawText(buf, 15, 178, 12, GOLD);
    sprintf(buf, "Tentmaking: %d", player->skills[SKILL_TENTMAKING].level);
    DrawText(buf, 15, 192, 12, GOLD);
    sprintf(buf, "Endurance: %d", player->skills[SKILL_ENDURANCE].level);
    DrawText(buf, 15, 206, 12, GOLD);
}

void DrawHUD(Player *player, World *world, bool isFirstPerson) {
    if (isFirstPerson) {
        int centerX = GetScreenWidth() / 2;
        int centerY = GetScreenHeight() / 2;
        DrawLine(centerX - 10, centerY, centerX + 10, centerY, GREEN);
        DrawLine(centerX, centerY - 10, centerX, centerY + 10, GREEN);
    }

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

    DrawRectangle(10, 40, 200, 50, Fade(DARKGRAY, 0.7f));
    DrawText("COORDINATES", 15, 45, 12, GOLD);
    char buf[64];
    sprintf(buf, "X: %d, Z: %d", player->position.x, player->position.z);
    DrawText(buf, 15, 60, 20, WHITE);

    DrawRectangle(10, 95, 220, 24, Fade(BLACK, 0.6f));
    DrawText(IsPlayerSeenByPharisee(world) ? "WATCHED: PHARISEE" : "WATCHED: CLEAR",
             18, 101, 14, IsPlayerSeenByPharisee(world) ? RED : GREEN);
    if (player->showMap) {
        int compassW = 400;
        int compassX = GetScreenWidth() / 2 - compassW / 2;
        int compassY = 30;
        DrawRectangle(compassX, compassY, compassW, 30, Fade(BLACK, 0.6f));
        DrawRectangleLines(compassX, compassY, compassW, 30, GOLD);
        DrawText("W", compassX + 5, compassY + 8, 15, WHITE);
        DrawText("E", compassX + compassW - 15, compassY + 8, 15, WHITE);

        for (int i = 0; i < (int)(sizeof(worldMap) / sizeof(City)); i++) {
            if (worldMap[i].coords.x < world->state.minX || worldMap[i].coords.x > world->state.maxX ||
                worldMap[i].coords.z < world->state.minZ || worldMap[i].coords.z > world->state.maxZ) {
                continue;
            }

            float dx = (float)worldMap[i].coords.x - player->lerpPosition.x;
            float dz = (float)worldMap[i].coords.z - player->lerpPosition.z;
            float dist = sqrtf(dx * dx + dz * dz);

            if (dist < 600.0f) {
                float relativeX = dx / 600.0f;
                int markerX = compassX + compassW / 2 + (int)(relativeX * (compassW / 2));

                if (markerX > compassX + 5 && markerX < compassX + compassW - 15) {
                    DrawRectangle(markerX - 2, compassY + 5, 4, 20, GOLD);
                    if (dist < 100.0f) DrawText(worldMap[i].name, markerX - 20, compassY - 20, 12, GOLD);
                }
            }
        }

        int mapW = 150;
        int mapH = 150;
        int mapX = GetScreenWidth() - mapW - 10;
        int mapY = 10;
        DrawRectangle(mapX, mapY, mapW, mapH, Fade(BLACK, 0.8f));
        DrawRectangleLines(mapX, mapY, mapW, mapH, GOLD);

        for (int i = 0; i < (int)(sizeof(worldMap) / sizeof(City)); i++) {
            if (worldMap[i].coords.x < world->state.minX || worldMap[i].coords.x > world->state.maxX ||
                worldMap[i].coords.z < world->state.minZ || worldMap[i].coords.z > world->state.maxZ) {
                continue;
            }
            int cityX = mapX + mapW / 2 + worldMap[i].coords.x / 5;
            int cityZ = mapY + mapH / 2 + worldMap[i].coords.z / 5;
            if (cityX > mapX && cityX < mapX + mapW && cityZ > mapY && cityZ < mapY + mapH) {
                DrawCircle(cityX, cityZ, 2, RED);
            }
        }

        int px = mapX + mapW / 2 + (int)player->lerpPosition.x / 5;
        int pz = mapY + mapH / 2 + (int)player->lerpPosition.z / 5;
        if (px > mapX && px < mapX + mapW && pz > mapY && pz < mapY + mapH) {
            DrawCircle(px, pz, 3, GREEN);
        }
    }

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
            DrawRectangle(10, 220, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Julius: You may visit friends in Sidon.", 18, 224, 14, SKYBLUE);
        }
    }

    if (world->state.hasSnake) {
        Vector3Int s = world->state.snakePos;
        if (abs(player->position.x - s.x) <= 6 && abs(player->position.z - s.z) <= 6) {
            DrawRectangle(10, 250, 360, 24, Fade(BLACK, 0.6f));
            DrawText("A viper strikes, but Paul is unharmed.", 18, 254, 14, ORANGE);
        }
    }

    if (!player->gameComplete && world->state.hasHouseArrest) {
        Vector3Int h = world->state.houseArrestPos;
        if (abs(player->position.x - h.x) <= 4 && abs(player->position.z - h.z) <= 4) {
            DrawRectangle(10, 280, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Press H to enter house arrest", 18, 284, 14, GOLD);
        }
    }

    if (player->gameComplete) {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.6f));
        DrawText("Arrived in Rome - House Arrest", w / 2 - 170, h / 2 - 10, 20, GOLD);
    }

    if (world->state.nearbyPreachNpcIndex != -1 &&
        !TextIsEqual(world->state.npcs[world->state.nearbyPreachNpcIndex].name, "Guard")) {
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

    if (player->worldMessageTimer > 0.0f && player->worldMessage[0] != '\0') {
        int msgW = 660;
        int msgX = GetScreenWidth() / 2 - msgW / 2;
        int msgY = GetScreenHeight() - 165;
        DrawRectangle(msgX, msgY, msgW, 42, Fade(BLACK, 0.8f));
        DrawRectangleLines(msgX, msgY, msgW, 42, SKYBLUE);
        DrawText(player->worldMessage, msgX + 14, msgY + 12, 18, WHITE);
    }

    int questStartX = GetScreenWidth() - 220;
    int questStartY = GetScreenHeight() - 450;
    DrawRectangle(questStartX, questStartY, 200, 90, Fade(BLUE, 0.4f));
    DrawRectangleLines(questStartX, questStartY, 200, 90, SKYBLUE);
    DrawText("QUEST LOG", questStartX + 10, questStartY + 10, 15, GOLD);

    if (player->activeQuestId != 0) {
        int qId = player->activeQuestId;
        DrawText(questDatabase[qId].title, questStartX + 10, questStartY + 35, 12, WHITE);
        DrawText(TextSubtext(questDatabase[qId].description, 0, 30), questStartX + 10, questStartY + 55, 10, GRAY);
    } else {
        DrawText("No active mission.", questStartX + 10, questStartY + 40, 12, GRAY);
    }
}

void DrawGuardDialogue(Player *player, World *world) {
    int guardIdx = GetNearbyGuardIndex(world, player);

    if (!player->guardDialogueActive) {
        if (guardIdx != -1 && !player->guardClearedForShip) {
            int promptW = 330;
            int promptX = GetScreenWidth() / 2 - promptW / 2;
            int promptY = GetScreenHeight() - 145;
            DrawRectangle(promptX, promptY, promptW, 42, Fade(BLACK, 0.8f));
            DrawRectangleLines(promptX, promptY, promptW, 42, GOLD);
            DrawText("Press E to speak with the Roman Guard", promptX + 16, promptY + 12, 18, GOLD);
        }
        return;
    }

    int boxX = 80;
    int boxY = GetScreenHeight() - 280;
    int boxW = GetScreenWidth() - 160;
    int boxH = 220;
    int textLeft = boxX + 25;

    DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.90f));
    DrawRectangleLines(boxX, boxY, boxW, boxH, GOLD);
    DrawText("ROMAN GUARD", textLeft, boxY + 20, 24, GOLD);

    if (player->guardClearedForShip) {
        DrawText("\"Very well... perhaps you are telling the truth.\"", textLeft, boxY + 75, 24, WHITE);
        DrawText("\"Make your way to the boat. You will sail for Malta.\"", textLeft, boxY + 115, 24, SKYBLUE);
        DrawText("Press E, ENTER, or SPACE to continue.", textLeft, boxY + 170, 18, LIGHTGRAY);
        return;
    }

    const GuardQuestion *q = &guardQuestions[player->guardDialogueStep];
    int questionY = boxY + 62;
    int questionFont = 18;
    int questionMaxWidth = boxW - 50;
    int questionLines = DrawWrappedTextBlock(q->line, textLeft, questionY, questionFont, questionMaxWidth, 4, WHITE);
    int referenceY = questionY + questionLines * (questionFont + 4) + 6;
    int answersY = referenceY + 28;
    int answerSpacing = 23;

    DrawText(q->reference, textLeft, referenceY, 16, SKYBLUE);
    DrawText(q->answers[0], textLeft + 15, answersY, 18, GOLD);
    DrawText(q->answers[1], textLeft + 15, answersY + answerSpacing, 18, GOLD);
    DrawText(q->answers[2], textLeft + 15, answersY + answerSpacing * 2, 18, GOLD);
    DrawText(q->answers[3], textLeft + 15, answersY + answerSpacing * 3, 18, GOLD);

    if (player->guardDialogueLastResult == 0) {
        DrawText("Roman Guard: \"That is not correct. Answer carefully.\"",
                 boxX + boxW - 470, boxY + boxH - 50, 18, RED);
    }
}
