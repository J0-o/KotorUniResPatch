#include "container_popup_scale.h"

namespace ContainerPopupScale {

namespace {

constexpr DWORD ContainerVtable = 0x007567E0;
constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;
constexpr int BaseWidth = 640;
constexpr int BaseHeight = 480;

bool safeReadDword(const void* address, DWORD& value) {
    __try {
        value = *reinterpret_cast<const DWORD*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0;
        return false;
    }
}

bool safeReadInt(const void* address, int& value) {
    __try {
        value = *reinterpret_cast<const int*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0;
        return false;
    }
}

bool hasUsefulRect(const Rect& rect) {
    return rect.width > 0 && rect.height > 0 &&
        rect.width < 8192 && rect.height < 8192;
}

int screenWidth() {
    int width = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenWidthAddress), width) || width <= 0) {
        return 800;
    }

    return width;
}

int screenHeight() {
    int height = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenHeightAddress), height) || height <= 0) {
        return 600;
    }

    return height;
}

bool isBaseResolution() {
    return screenWidth() == 800 && screenHeight() == 600;
}

int scaleValue(int value, int target, int source) {
    if (target <= 0 || source <= 0) {
        return value;
    }

    return static_cast<int>((static_cast<long long>(value) * target) / source);
}

int menuWidth() {
    int width = scaleValue(BaseWidth, screenHeight(), BaseHeight);
    if (width > screenWidth()) {
        width = screenWidth();
    }

    return width;
}

Rect scaledChildRect(const Rect& rect) {
    const int width = menuWidth();
    return {
        scaleValue(rect.left, width, BaseWidth),
        scaleValue(rect.top, screenHeight(), BaseHeight),
        scaleValue(rect.width, width, BaseWidth),
        scaleValue(rect.height, screenHeight(), BaseHeight),
    };
}

Rect scaledRootRect(const Rect& rect) {
    return scaledChildRect(rect);
}

void callControlSetRect(char* control, const Rect& rect) {
    if (!control) {
        return;
    }

    __try {
        DWORD vtable = *reinterpret_cast<DWORD*>(control);
        DWORD setRect = *reinterpret_cast<DWORD*>(vtable + 4);
        if (setRect != 0) {
            typedef void(__thiscall *SetRectFn)(void*, const Rect*);
            reinterpret_cast<SetRectFn>(setRect)(control, &rect);
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    __try {
        *reinterpret_cast<Rect*>(control + sizeof(DWORD)) = rect;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

void scaleControl(char* control) {
    if (!control) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (hasUsefulRect(*rect)) {
        callControlSetRect(control, scaledChildRect(*rect));
    }
}

void scalePanelControls(char* panel) {
    DWORD childrenData = 0;
    DWORD childrenSize = 0;
    if (!safeReadDword(panel + 0x20, childrenData) ||
        !safeReadDword(panel + 0x24, childrenSize) ||
        childrenData == 0 ||
        childrenSize > 64) {
        return;
    }

    for (DWORD i = 0; i < childrenSize; ++i) {
        DWORD child = 0;
        if (safeReadDword(reinterpret_cast<const void*>(childrenData + (i * sizeof(DWORD))), child) &&
            child != 0) {
            scaleControl(reinterpret_cast<char*>(child));
        }
    }
}

}

void scaleContainerPanel(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner || isBaseResolution()) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || vtable != ContainerVtable) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    scalePanelControls(owner);
    callControlSetRect(owner, scaledRootRect(*root));
}

}
