#include "combat.h"
#include "platform_input.h"
#include "scripture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void StartCombat(CombatSession *session, const char *opponent) {
    session->active = true;
    session->opponentName = opponent;
    session->opponentSpirit = 100;
    session->opponentMaxSpirit = 100;
    session->attackInterval = 5.0f; // 5 seconds to answer
    session->attackTimer = session->attackInterval;

    // Pull real verses from the database for variety
    // For now, let's pick 4 verses, one is "correct" for the challenge
    session->lastQuote = "How can you say Jesus is the Christ? The Law says the Messiah will remain forever!";
    
    // Pick 4 random-ish verses from the first 500
    for (int i = 0; i < 4; i++) {
        int idx = GetRandomValue(0, 500);
        session->options[i] = actsDatabase[idx].text;
    }
    
    // Set one to a more relevant verse (e.g., Acts 2:24)
    session->correctOption = GetRandomValue(0, 3);
    session->options[session->correctOption] = "Acts 2:24: 'But God raised him up, having loosed the pangs of death...'";
}

void UpdateCombat(CombatSession *session, Player *player) {
    if (!session->active) return;

    session->attackTimer -= GetFrameTime();

    if (session->attackTimer <= 0) {
        // Opponent attacks Paul's Spirit!
        player->spirit -= 10;
        session->attackTimer = session->attackInterval; // Reset timer
    }

    int choice = -1;
    if (IsKeyPressed(KEY_ONE)) choice = 0;
    else if (IsKeyPressed(KEY_TWO)) choice = 1;
    else if (IsKeyPressed(KEY_THREE)) choice = 2;
    else if (IsKeyPressed(KEY_FOUR)) choice = 3;

    if (choice != -1) {
        if (choice == session->correctOption) {
            session->opponentSpirit -= 35;
            player->skills[SKILL_ORATORY].currentXp += 50;
            // Generate next challenge or win
            if (session->opponentSpirit > 0) {
                session->attackTimer = session->attackInterval; // Reset timer on success
                // Rotate options (ideally random again)
                session->correctOption = GetRandomValue(0, 3);
                for (int i = 0; i < 4; i++) {
                    int idx = GetRandomValue(0, 500);
                    session->options[i] = actsDatabase[idx].text;
                }
                session->options[session->correctOption] = "Acts 4:12: 'There is salvation in no one else...'";
            }
        } else {
            // Wrong answer! Paul takes Spirit damage
            player->spirit -= 15;
            session->attackTimer = session->attackInterval; // Reset timer to give another chance
        }
    }

    if (session->opponentSpirit <= 0) {
        session->active = false;
        // Winner! Give some Oratory XP
    }
    if (player->spirit <= 0) {
        session->active = false;
        // Game Over or Faint logic
        player->spirit = 20; // For now, just reset a bit
        player->position = (Vector3Int){0,0,0}; // Teleport to Jerusalem
        player->target = player->position;
        player->lerpPosition = (Vector3){0,0,0};
    }
}

void DrawCombatUI(CombatSession *session, int screenWidth, int screenHeight) {
    if (!session->active) return;

    // Overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));

    // Opponent Panel
    DrawRectangle(screenWidth / 2 - 300, 50, 600, 150, DARKGRAY);
    DrawRectangleLines(screenWidth / 2 - 300, 50, 600, 150, MAROON);
    DrawText(session->opponentName, screenWidth / 2 - 280, 60, 20, GOLD);

    const char *quoteText = session->lastQuote;
    char qTruncated[95];
    if (TextLength(quoteText) > 90) {
        TextCopy(qTruncated, TextSubtext(quoteText, 0, 87));
        strcat(qTruncated, "...");
    } else {
        TextCopy(qTruncated, quoteText);
    }
    DrawText(qTruncated, screenWidth / 2 - 280, 90, 16, WHITE);

    // Opponent Spirit Bar
    DrawRectangle(screenWidth / 2 - 280, 130, 560, 20, BLACK);
    float pct = (float)session->opponentSpirit / (float)session->opponentMaxSpirit;
    DrawRectangle(screenWidth / 2 - 278, 132, (int)(556 * pct), 16, PURPLE);

    // ATTACK TIMER BAR (Top of screen)
    float timerPct = session->attackTimer / session->attackInterval;
    DrawRectangle(0, 0, screenWidth, 10, BLACK);
    DrawRectangle(0, 0, (int)(screenWidth * timerPct), 10, ORANGE);
    DrawText("OPPONENT PREPARING COUNTER-ARGUMENT", screenWidth / 2 - 150, 15, 12, ORANGE);

    // Dialogue Options
    int boxY = screenHeight - 250;
    DrawRectangle(screenWidth / 2 - 400, boxY, 800, 200, Fade(BROWN, 0.9f));
    DrawRectangleLines(screenWidth / 2 - 400, boxY, 800, 200, GOLD);

    for (int i = 0; i < 4; i++) {
        char buf[512];
        const char *origText = session->options[i];
        char truncated[95];
        if (TextLength(origText) > 90) {
            TextCopy(truncated, TextSubtext(origText, 0, 87));
            strcat(truncated, "...");
        } else {
            TextCopy(truncated, origText);
        }

        sprintf(buf, "%d. %s", i + 1, truncated);
        Color col = GOLD;
        DrawText(buf, screenWidth / 2 - 380, boxY + 20 + (i * 40), 16, col);
    }
}
