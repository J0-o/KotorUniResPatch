#include "popup_dialog_scale_test.h"

extern "C" void __cdecl scaleCenteredPopup(void* owner, DWORD* returnAddressSlot) {
    __try {
        PopupDialogScaleTest::scaleCenteredPopup(owner, returnAddressSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleLayoutPopup(void* owner) {
    __try {
        PopupDialogScaleTest::scaleLayoutPopup(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleLateResolutionPopup(void* owner) {
    __try {
        PopupDialogScaleTest::scaleLateResolutionPopup(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleStatusSummarySetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    __try {
        PopupDialogScaleTest::scaleStatusSummarySetRect(control, returnAddressSlot, rectPointerSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleMessageBoxButtonSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    __try {
        PopupDialogScaleTest::scaleMessageBoxButtonSetRect(control, returnAddressSlot, rectPointerSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleMessageBoxAfterFix(void* owner) {
    __try {
        PopupDialogScaleTest::scaleMessageBoxAfterFix(owner);
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
