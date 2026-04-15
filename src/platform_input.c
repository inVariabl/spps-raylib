#include "platform_input.h"

#undef IsKeyDown
#undef IsKeyPressed

#if SPPS_PLATFORM_WEB
#include <emscripten/emscripten.h>

static bool s_webDown[512] = {0};
static bool s_webPressed[512] = {0};

EMSCRIPTEN_KEEPALIVE void SPPS_WebSetKeyState(int key, int isDown) {
    if (key < 0 || key >= (int)(sizeof(s_webDown) / sizeof(s_webDown[0]))) return;

    bool down = (isDown != 0);
    if (down && !s_webDown[key]) s_webPressed[key] = true;
    s_webDown[key] = down;
}

EMSCRIPTEN_KEEPALIVE void SPPS_WebPulseKey(int key) {
    if (key < 0 || key >= (int)(sizeof(s_webPressed) / sizeof(s_webPressed[0]))) return;
    s_webPressed[key] = true;
}

EMSCRIPTEN_KEEPALIVE void SPPS_WebResetInputs(void) {
    for (int i = 0; i < (int)(sizeof(s_webDown) / sizeof(s_webDown[0])); i++) {
        s_webDown[i] = false;
        s_webPressed[i] = false;
    }
}
#endif

bool SppsIsKeyDown(int key) {
#if SPPS_PLATFORM_WEB
    if (key >= 0 && key < (int)(sizeof(s_webDown) / sizeof(s_webDown[0])) && s_webDown[key]) return true;
#endif
    return IsKeyDown(key);
}

bool SppsIsKeyPressed(int key) {
#if SPPS_PLATFORM_WEB
    if (key >= 0 && key < (int)(sizeof(s_webPressed) / sizeof(s_webPressed[0])) && s_webPressed[key]) return true;
#endif
    return IsKeyPressed(key);
}

void SppsInputFrameEnd(void) {
#if SPPS_PLATFORM_WEB
    for (int i = 0; i < (int)(sizeof(s_webPressed) / sizeof(s_webPressed[0])); i++) {
        s_webPressed[i] = false;
    }
#endif
}
