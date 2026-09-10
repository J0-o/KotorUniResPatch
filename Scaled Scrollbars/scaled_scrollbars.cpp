#include <windows.h>

#include "../Common/ResolutionScale.h"

namespace {

constexpr DWORD ScrollbarWidthOffset = 0x110;
constexpr int BaseScrollbarWidth = 16;
constexpr int MaxTrackedListboxes = 512;

struct TrackedListbox {
    void* object;
    DWORD vtable;
};

TrackedListbox trackedListboxes[MaxTrackedListboxes] = {};

void rememberListbox(void* object) {
    __try {
        const DWORD vtable = *reinterpret_cast<DWORD*>(object);
        for (TrackedListbox& tracked : trackedListboxes) {
            if (tracked.object == object) {
                tracked.vtable = vtable;
                return;
            }
            if (!tracked.object) {
                tracked = { object, vtable };
                return;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

void applyScrollbarWidth(void* listbox, const UniversalScaleState& scale) {
    char* object = static_cast<char*>(listbox);
    int* width = reinterpret_cast<int*>(object + ScrollbarWidthOffset);
    if (*width != 0) {
        *width = scaleUiValue(BaseScrollbarWidth, scale);
    }
}

void __cdecl refreshTrackedScrollbars() {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale) {
        return;
    }
    for (TrackedListbox& tracked : trackedListboxes) {
        if (!tracked.object) {
            continue;
        }
        __try {
            if (*reinterpret_cast<DWORD*>(tracked.object) != tracked.vtable) {
                tracked = {};
                continue;
            }
            applyScrollbarWidth(tracked.object, *scale);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            tracked = {};
        }
    }
}

}

extern "C" void __cdecl scaleScrollbarWidth(void* listboxPtr) {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!listboxPtr || !scale) {
        return;
    }

    __try {
        rememberListbox(listboxPtr);
        applyScrollbarWidth(listboxPtr, *scale);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl refreshResolutionDependentUi() {
    refreshTrackedScrollbars();
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
