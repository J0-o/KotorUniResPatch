#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Memory patch helpers

bool writeMemory(void* address, const void* replacement, size_t size) {
    __try {
        DWORD oldProtect = 0;
        if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }

        CopyMemory(address, replacement, size);
        FlushInstructionCache(GetCurrentProcess(), address, size);
        DWORD ignored = 0;
        VirtualProtect(address, size, oldProtect, &ignored);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void writeInt(int address, int value) {
    writeMemory(reinterpret_cast<void*>(address), &value, sizeof(value));
}

void setControlRect(void* control, const Rect& rect) {
    if (!control) {
        return;
    }

    typedef void(__thiscall *SetRectFn)(void*, const Rect*);

    DWORD vtable = 0;
    DWORD setRectAddress = 0;
    if (!safeReadDword(control, vtable) || !vtable) {
        return;
    }

    if (!safeReadDword(reinterpret_cast<const void*>(vtable + 0x04), setRectAddress) || !setRectAddress) {
        return;
    }

    SetRectFn setRect = reinterpret_cast<SetRectFn>(setRectAddress);
    setRect(control, &rect);
}


}
