#include "popup_dialog_scale_test.h"
#include "../Common/ResolutionScale.h"

extern "C" void __cdecl scaleCenteredPopup(void* owner, DWORD* returnAddressSlot) {
    __try {
        PopupDialogScaleTest::scaleCenteredPopup(owner, returnAddressSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleBarkPopup(void* owner) {
    __try {
        PopupDialogScaleTest::scaleLayoutPopup(owner, true);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scalePausePopup(void* owner) {
    __try {
        PopupDialogScaleTest::scaleLayoutPopup(owner, false);
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

extern "C" void __cdecl refreshLateResolutionPopup(void* owner) {
    __try {
        PopupDialogScaleTest::refreshLateResolutionPopup(owner);
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

extern "C" void __cdecl refreshResolutionDependentUi() {
    __try {
        PopupDialogScaleTest::refreshTrackedPopups();
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
