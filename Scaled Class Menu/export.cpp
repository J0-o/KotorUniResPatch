#include "class_selection_layout_constants_test.h"

namespace {

constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;

bool isBaseResolution() {
    return *reinterpret_cast<int*>(ScreenWidthAddress) == 800 &&
        *reinterpret_cast<int*>(ScreenHeightAddress) == 600;
}

}

extern "C" void __cdecl patchClassSelectionLayoutRects(void* ownerStackSlot, void* currentSlotMarker, void* baseRectStack, void* wrapperRectStack) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        ClassSelectionLayoutConstantsTest::patchClassSelectionLayoutRects(ownerStackSlot, currentSlotMarker, baseRectStack, wrapperRectStack);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl patchInitialClassSelectionRects(void* owner) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        ClassSelectionLayoutConstantsTest::patchInitialClassSelectionRects(owner);
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
