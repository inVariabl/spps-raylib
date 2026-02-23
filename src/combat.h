#ifndef COMBAT_H
#define COMBAT_H

#include "player.h"

typedef struct {
    bool active;
    const char *opponentName;
    int opponentSpirit;
    int opponentMaxSpirit;
    const char *lastQuote;
    const char *options[4];
    int correctOption;
    float attackTimer;
    float attackInterval;
} CombatSession;

void StartCombat(CombatSession *session, const char *opponent);
void UpdateCombat(CombatSession *session, Player *player);
void DrawCombatUI(CombatSession *session, int screenWidth, int screenHeight);

#endif // COMBAT_H
