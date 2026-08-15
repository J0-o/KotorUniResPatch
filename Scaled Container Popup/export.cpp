#include "container_popup_scale_test.h"

extern "C" void __cdecl scaleContainerPanel(void* owner) {
    __try {
        ContainerPopupScaleTest::scaleContainerPanel(owner);
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
