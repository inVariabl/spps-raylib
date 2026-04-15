#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

#include "common.h"

bool SppsIsKeyDown(int key);
bool SppsIsKeyPressed(int key);
void SppsInputFrameEnd(void);

#if SPPS_PLATFORM_WEB
void SPPS_WebSetKeyState(int key, int isDown);
void SPPS_WebPulseKey(int key);
void SPPS_WebResetInputs(void);
#endif

#define IsKeyDown SppsIsKeyDown
#define IsKeyPressed SppsIsKeyPressed

#endif
