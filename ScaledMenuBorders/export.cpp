#include "menu_background_shift_test.h"

extern "C" void __cdecl center800x600backDynamic(void* parent) {
    __try {
        MenuBackgroundShiftTest::centerMenuBackgroundDynamic(parent);
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
