#include "class_selection_layout_constants_test.h"

extern "C" void __cdecl patchClassSelectionLayoutRects(void* ownerStackSlot, void* currentSlotMarker, void* baseRectStack, void* wrapperRectStack) {
    __try {
        ClassSelectionLayoutConstantsTest::patchClassSelectionLayoutRects(ownerStackSlot, currentSlotMarker, baseRectStack, wrapperRectStack);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl patchInitialClassSelectionRects(void* owner) {
    __try {
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
