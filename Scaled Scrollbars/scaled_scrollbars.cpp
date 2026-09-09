#include <windows.h>

#include "../Common/ResolutionScale.h"

namespace {

constexpr DWORD ScrollbarWidthOffset = 0x110;
constexpr int BaseScrollbarWidth = 16;

}

extern "C" void __cdecl scaleScrollbarWidth(void* listboxPtr) {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!listboxPtr || !scale) {
        return;
    }

    __try {
        char* listbox = static_cast<char*>(listboxPtr);
        int* width = reinterpret_cast<int*>(listbox + ScrollbarWidthOffset);
        if (*width != 0) {
            *width = scaleUiValue(BaseScrollbarWidth, *scale);
        }
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
