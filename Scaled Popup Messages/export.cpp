#include "popup_dialog_scale_test.h"

extern "C" void __cdecl scalePopupDialogPanel(void* owner) {
    __try {
        PopupDialogScaleTest::scalePopupDialogPanel(owner);
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

extern "C" void __cdecl scaleMessageBoxLabelSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    __try {
        PopupDialogScaleTest::scaleMessageBoxLabelSetRect(control, returnAddressSlot, rectPointerSlot);
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
