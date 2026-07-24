#include "scaled_menu.h"

namespace {

constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;

bool isBaseResolution() {
    return *reinterpret_cast<int*>(ScreenWidthAddress) == 800 &&
        *reinterpret_cast<int*>(ScreenHeightAddress) == 600;
}

}

extern "C" void __cdecl scaleMenuPanelTree(void* panel) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        MenuScale::scaleMenuPanelTree(panel);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scalePazaakGameCards(void* pazaakGame) {
    __try {
        if (isBaseResolution()) {
            return;
        }
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
