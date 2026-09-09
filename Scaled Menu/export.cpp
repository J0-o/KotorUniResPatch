#include "scaled_menu.h"

extern "C" void __cdecl scaleMenuPanelTree(void* panel) {
    __try {
        MenuScale::scaleMenuPanelTree(panel);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scalePazaakGameCards(void* pazaakGame) {
    __try {
        MenuScale::scalePazaakGameCards(pazaakGame);
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
