#include "ui.h"
#include "platform_input.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

static bool PlayerHasItem(const Player *player, int itemId) {
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (player->inventory[i].itemId == itemId) {
            return true;
        }
    }
    return false;
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

static int GetNearbyGuardIndex(World *world, Player *player) {
    for (int i = 0; i < 20; i++) {
        NPC *npc = &world->state.npcs[i];
        if (!npc->active || !TextIsEqual(npc->name, "Guard")) continue;

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
            size_t lineSize = strlen(line);
            size_t wordSize = strlen(word);

            if (lineLen == 0) {
                snprintf(testLine, sizeof(testLine), "%s", word);
            } else {
                size_t needed = lineSize + 1 + wordSize + 1;
                if (needed >= sizeof(testLine)) {
                    DrawText(line, x, y + lineCount * (fontSize + lineSpacing), fontSize, color);
                    lineCount++;
                    snprintf(line, sizeof(line), "%s", word);
                    lineLen = (int)strlen(line);
                    wordLen = 0;
                    if (c == '\n') {
                        DrawText(line, x, y + lineCount * (fontSize + lineSpacing), fontSize, color);
                        lineCount++;
                        line[0] = '\0';
                        lineLen = 0;
                    }
                    if (c == '\0') break;
                    continue;
                }

                memcpy(testLine, line, lineSize);
                testLine[lineSize] = ' ';
                memcpy(testLine + lineSize + 1, word, wordSize + 1);
            }

            if (MeasureText(testLine, fontSize) > maxWidth && lineLen > 0) {
                DrawText(line, x, y + lineCount * (fontSize + lineSpacing), fontSize, color);
                lineCount++;
                snprintf(line, sizeof(line), "%s", word);
                lineLen = (int)strlen(line);
            } else {
                if (lineLen == 0) snprintf(line, sizeof(line), "%s", word);
                else {
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

static void DrawWantedStarIcon(Vector2 center, float radius, Color color) {
    float triHeight = radius;
    float halfWidth = radius * 0.8660254f;

    Vector2 upA = {center.x, center.y - triHeight};
    Vector2 upB = {center.x - halfWidth, center.y + triHeight * 0.5f};
    Vector2 upC = {center.x + halfWidth, center.y + triHeight * 0.5f};

    Vector2 downA = {center.x, center.y + triHeight};
    Vector2 downB = {center.x - halfWidth, center.y - triHeight * 0.5f};
    Vector2 downC = {center.x + halfWidth, center.y - triHeight * 0.5f};

    DrawLineEx(upA, upB, 2.0f, color);
    DrawLineEx(upB, upC, 2.0f, color);
    DrawLineEx(upC, upA, 2.0f, color);

    DrawLineEx(downA, downB, 2.0f, color);
    DrawLineEx(downB, downC, 2.0f, color);
    DrawLineEx(downC, downA, 2.0f, color);
}

static void GetQuestLogText(const Player *player, const World *world,
                            const char **title, const char **line1, const char **line2) {
    *title = "Current Objective";
    *line1 = "Keep moving forward.";
    *line2 = "";

    if (world->state.worldId == WORLD_JUDEA) {
        *title = "Jerusalem";
        if (!PlayerHasItem(player, 5)) {
            *line1 = "Collect the scroll nearby.";
            *line2 = "Prepare to leave Jerusalem.";
        } else if (!player->guardClearedForShip) {
            *line1 = "Speak with the Roman guard.";
            *line2 = "Earn passage from Sidon.";
        } else {
            *line1 = "Make your way to the ship.";
            *line2 = "Travel when you are ready.";
        }
        return;
    }

    if (world->state.worldId == WORLD_MALTA) {
        *title = "Malta";
        if (!IsMaltaFireLit(world)) {
            *line1 = "Click the fire to light it.";
            *line2 = "Stay with the islanders.";
        } else if (!IsMaltaSnakeResolved(world)) {
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
        int believersMet = CountRomeBelieversMet(player);
        if (believersMet < 3) {
            *line1 = "Speak with the believers on the road.";
            *line2 = "Then report to the centurion.";
        } else if (!player->romeCenturionMet) {
            *line1 = "Meet the Roman centurion.";
            *line2 = "He will place you under guard in Rome.";
        } else if (!player->gameComplete) {
            *line1 = "Proceed to the house in Rome.";
            *line2 = "Your journey is nearly complete.";
        } else {
            *line1 = "Paul has reached Rome.";
            *line2 = "The journey ends under house arrest.";
        }
    }
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
        DrawWantedStarIcon((Vector2){(float)(starsX + 10 + i * 24), (float)(starsY + 10)}, 8.0f, starColor);
    }

    // Location Display
    DrawRectangle(10, 40, 200, 50, Fade(DARKGRAY, 0.7f));
    DrawText("COORDINATES", 15, 45, 12, GOLD);
    char buf[64];
    sprintf(buf, "X: %d, Z: %d", player->position.x, player->position.z);
    DrawText(buf, 15, 60, 20, WHITE);

    DrawRectangle(10, 95, 220, 24, Fade(BLACK, 0.6f));
    DrawText(IsPlayerSeenByPharisee(world) ? "WATCHED: PHARISEE" : "WATCHED: CLEAR",
             18, 101, 14, IsPlayerSeenByPharisee(world) ? RED : GREEN);

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
            DrawRectangle(10, 160, 360, 24, Fade(BLACK, 0.6f));
            DrawText("Julius: You may visit friends in Sidon.", 18, 164, 14, SKYBLUE);
        }
    }

    if (HasDecorationType(world, DECO_SNAKE)) {
        Vector3Int s = world->state.snakePos;
        if (abs(player->position.x - s.x) <= 6 && abs(player->position.z - s.z) <= 6) {
            DrawRectangle(10, 190, 360, 24, Fade(BLACK, 0.6f));
            DrawText("A viper strikes, but Paul is unharmed.", 18, 194, 14, ORANGE);
        }
    }

    if (player->gameComplete) {
        int w = GetScreenWidth();
        int h = GetScreenHeight();
        DrawRectangle(0, 0, w, h, Fade(BLACK, 0.6f));
        DrawText("Arrived in Rome - House Arrest", w / 2 - 170, h / 2 - 10, 20, GOLD);
    }

    if (world->state.nearbyPreachNpcIndex != -1) {
        NPC *npc = &world->state.npcs[world->state.nearbyPreachNpcIndex];
        int promptW = 360;
        int promptX = GetScreenWidth() / 2 - promptW / 2;
        int promptY = GetScreenHeight() - 145;
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
        int msgY = GetScreenHeight() - 195;
        DrawRectangle(msgX, msgY, msgW, 42, Fade(BLACK, 0.8f));
        DrawRectangleLines(msgX, msgY, msgW, 42, SKYBLUE);
        DrawText(player->worldMessage, msgX + 14, msgY + 12, 18, WHITE);
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

    if (SPPS_PLATFORM_WEB) {
        int panelX = 10;
        int panelY = GetScreenHeight() - 118;
        int panelW = 470;
        int panelH = 62;
        DrawRectangle(panelX, panelY, panelW, panelH, Fade(BLACK, 0.72f));
        DrawRectangleLines(panelX, panelY, panelW, panelH, Fade(SKYBLUE, 0.8f));
        DrawText("WEB CONTROLS", panelX + 12, panelY + 8, 16, GOLD);
        DrawText("Click the canvas first. Use F1 and F3 directly in the browser.", panelX + 12, panelY + 28, 15, RAYWHITE);
        DrawText("Arrow keys steer the ship. Tab switches the voyage interior.", panelX + 12, panelY + 44, 15, Fade(RAYWHITE, 0.88f));
    }
}

void DrawGuardDialogue(Player *player, World *world) {
    int guardIdx = GetNearbyGuardIndex(world, player);

    if (!player->guardDialogueActive) {
        if (guardIdx != -1 && !player->guardClearedForShip) {
            int promptW = 330;
            int promptX = GetScreenWidth() / 2 - promptW / 2;
            int promptY = GetScreenHeight() - 115;
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
    bool saveEnabled = !SPPS_PLATFORM_WEB;
    DrawRectangleRec(saveBtn, saveEnabled ? (hovered ? GREEN : DARKGRAY) : GRAY);
    DrawText(saveEnabled ? "SAVE" : "BROWSER", saveBtn.x + (saveEnabled ? 30 : 20), saveBtn.y + 8, 10, WHITE);
    
    if (saveEnabled && hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        FILE *f = fopen("shader_settings.txt", "w");
        if (f) {
            fprintf(f, "%f %f %f\n", settings->lightDir.x, settings->lightDir.y, settings->lightDir.z);
            fprintf(f, "%d %d %d\n", settings->lightColor.r, settings->lightColor.g, settings->lightColor.b);
            fprintf(f, "%f\n", settings->ambient);
            fprintf(f, "%f\n", settings->shadowBias);
            fclose(f);
        }
    }

    if (SPPS_PLATFORM_WEB) {
        DrawText("Shader saving is disabled in the browser build.", x + 140, startY + spacing*9 + 8, 10, LIGHTGRAY);
    }
}
