#include "scaled_hud.h"

extern "C" void __cdecl scaleHudControls(void* hud) {
    __try {
        HudScale::scaleHudControls(hud);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleHudActionDescription(void* hud) {
    __try {
        HudScale::scaleHudActionDescription(hud);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleAreaTransitionPrompt(void* prompt) {
    __try {
        HudScale::scaleAreaTransitionPrompt(prompt);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(reserved);
    return TRUE;
}
