#include "scaled_panels.h"

extern "C" void __cdecl scaleQuickOrCustomPanel(void* owner) {
    __try {
        ScaledPanels::scaleQuickOrCustomPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleQuickPanel(void* owner) {
    __try {
        ScaledPanels::scaleQuickPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleLevelUpPanel(void* owner) {
    __try {
        ScaledPanels::scaleLevelUpPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleCustomPanel(void* owner) {
    __try {
        ScaledPanels::scaleCustomPanel(owner);
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
