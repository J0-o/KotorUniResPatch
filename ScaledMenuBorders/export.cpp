#include "menu_background_shift_test.h"
#include "../Common/ResolutionScale.h"

extern "C" void __cdecl center800x600backDynamic(void* parent) {
    __try {
        MenuBackgroundShiftTest::centerMenuBackgroundDynamic(parent);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl refreshResolutionDependentUi() {
    __try {
        MenuBackgroundShiftTest::refreshMenuBackgrounds();
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
