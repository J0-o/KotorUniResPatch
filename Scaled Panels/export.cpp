#include "scaled_panels.h"
#include "../Common/ResolutionScale.h"

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

extern "C" void __cdecl refreshResolutionDependentUi() {
    __try {
        ScaledPanels::refreshScaledPanels();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reserved);
    if (reason == DLL_PROCESS_ATTACH) {
        ResolutionScale::subscribe(refreshResolutionDependentUi);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        ResolutionScale::unsubscribe(refreshResolutionDependentUi);
    }
    return TRUE;
}
