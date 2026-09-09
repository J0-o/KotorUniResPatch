#include "scaled_panels.h"
#include "../Common/ResolutionScale.h"

namespace ScaledPanels {

namespace {

constexpr DWORD QuickOrCustomVtable = 0x00759710;
constexpr DWORD QuickPanelVtable = 0x00759668;
constexpr DWORD LevelUpPanelVtable = 0x00759568;
constexpr DWORD CustomPanelVtable = 0x007595E0;

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

bool hasUsefulRect(const Rect& rect) {
    return rect.width > 0 && rect.height > 0 &&
        rect.width < 8192 && rect.height < 8192;
}

Rect scaledChildRect(const Rect& rect, const UniversalScaleState& scale) {
    return {
        scaleUiValue(rect.left, scale),
        scaleUiValue(rect.top, scale),
        scaleUiValue(rect.width, scale),
        scaleUiValue(rect.height, scale),
    };
}

Rect scaledRootRect(const Rect& rect, const UniversalScaleState& scale) {
    return scaledChildRect(rect, scale);
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

void scaleControl(char* control, const UniversalScaleState& scale) {
    if (!control) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (hasUsefulRect(*rect)) {
        callControlSetRect(control, scaledChildRect(*rect, scale));
    }
}

void scalePanelControls(char* panel, const UniversalScaleState& scale) {
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
            scaleControl(reinterpret_cast<char*>(child), scale);
        }
    }
}

void scaleSmallRootPanel(void* ownerPtr, DWORD expectedVtable) {
    char* owner = static_cast<char*>(ownerPtr);
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!owner || !scale || isIdentityUiScale(*scale)) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || vtable != expectedVtable) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    scalePanelControls(owner, *scale);
    callControlSetRect(owner, scaledRootRect(*root, *scale));
}

}

void scaleQuickOrCustomPanel(void* owner) {
    scaleSmallRootPanel(owner, QuickOrCustomVtable);
}

void scaleQuickPanel(void* owner) {
    scaleSmallRootPanel(owner, QuickPanelVtable);
}

void scaleLevelUpPanel(void* owner) {
    scaleSmallRootPanel(owner, LevelUpPanelVtable);
}

void scaleCustomPanel(void* owner) {
    scaleSmallRootPanel(owner, CustomPanelVtable);
}

}
