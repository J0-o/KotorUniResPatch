#include "scaled_panels.h"
#include "../Common/ResolutionScale.h"

namespace ScaledPanels {

namespace {

constexpr DWORD QuickOrCustomVtable = 0x00759710;
constexpr DWORD QuickPanelVtable = 0x00759668;
constexpr DWORD LevelUpPanelVtable = 0x00759568;
constexpr DWORD CustomPanelVtable = 0x007595E0;
constexpr DWORD MaxSnapshotChildren = 64;

struct ControlSnapshot {
    char* control;
    Rect rect;
};

struct PanelSnapshot {
    char* owner;
    DWORD vtable;
    Rect root;
    ControlSnapshot children[MaxSnapshotChildren];
    DWORD childCount;
};

PanelSnapshot livePanels[4] = {};

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

PanelSnapshot* snapshotForVtable(DWORD vtable) {
    const DWORD vtables[] = {
        QuickOrCustomVtable, QuickPanelVtable, LevelUpPanelVtable,
        CustomPanelVtable,
    };
    for (int i = 0; i < 4; ++i) {
        if (vtables[i] == vtable) {
            return &livePanels[i];
        }
    }
    return nullptr;
}

bool capturePanel(char* owner, DWORD vtable, PanelSnapshot& snapshot) {
    DWORD childrenData = 0;
    DWORD childrenSize = 0;
    Rect root = {};
    if (!safeReadDword(owner + 0x20, childrenData) ||
        !safeReadDword(owner + 0x24, childrenSize) ||
        childrenSize > MaxSnapshotChildren) {
        return false;
    }
    __try {
        root = *reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
    if (!hasUsefulRect(root)) {
        return false;
    }

    snapshot = {};
    snapshot.owner = owner;
    snapshot.vtable = vtable;
    snapshot.root = root;
    for (DWORD i = 0; i < childrenSize; ++i) {
        DWORD childValue = 0;
        if (!safeReadDword(reinterpret_cast<void*>(childrenData + i * sizeof(DWORD)),
                           childValue) || childValue == 0) {
            continue;
        }
        char* child = reinterpret_cast<char*>(childValue);
        __try {
            Rect rect = *reinterpret_cast<Rect*>(child + sizeof(DWORD));
            if (hasUsefulRect(rect)) {
                snapshot.children[snapshot.childCount++] = { child, rect };
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
    return true;
}

void applyPanel(PanelSnapshot& snapshot, const UniversalScaleState& scale) {
    DWORD vtable = 0;
    if (!snapshot.owner ||
        !safeReadDword(snapshot.owner, vtable) || vtable != snapshot.vtable) {
        snapshot = {};
        return;
    }
    for (DWORD i = 0; i < snapshot.childCount; ++i) {
        callControlSetRect(snapshot.children[i].control,
            scaledChildRect(snapshot.children[i].rect, scale));
    }
    callControlSetRect(snapshot.owner, scaledRootRect(snapshot.root, scale));
}

void scaleSmallRootPanel(void* ownerPtr, DWORD expectedVtable) {
    char* owner = static_cast<char*>(ownerPtr);
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!owner || !scale) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || vtable != expectedVtable) {
        return;
    }

    PanelSnapshot* snapshot = snapshotForVtable(expectedVtable);
    if (!snapshot || !capturePanel(owner, expectedVtable, *snapshot)) {
        return;
    }

    applyPanel(*snapshot, *scale);
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

void refreshScaledPanels() {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale) {
        return;
    }
    for (PanelSnapshot& panel : livePanels) {
        applyPanel(panel, *scale);
    }
}

}
