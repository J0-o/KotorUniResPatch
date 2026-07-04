#include "scaled_hud.h"

namespace HudScale {

namespace {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;
constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;

constexpr DWORD MinimapOffsets[] = {
    0x5CC0, // map_border_label
    0x5E00, // map_label
    0x5F40, // arrow_label
    0x6098  // mini_map_button
};

constexpr DWORD HudControlOffsets[] = {
    0x1BC8, 0x1D08, 0x1E48,             // moulding_labels
    0x6E94,                             // clear button
    0x735C, 0x749C,                     // combat mode message
    0xB00C, 0xB1D0, 0xB394, 0xB558,     // menu buttons
    0xB71C, 0xB8E0, 0xBAA4, 0xBC68,
    0xBE2C, 0xBF6C,                     // menu background/arrow margin
    0xC0AC, 0xC360, 0xC614              // toggles
};

constexpr DWORD ActionDescriptionControlOffsets[] = {
    0xA1D4, 0xA314
};
constexpr DWORD ActionDescriptionBottomOffset = 0xA454;

constexpr DWORD QueuedActionMarkerOffsets[] = {
    0x625C, 0x639C, 0x64DC, 0x661C
};

constexpr DWORD QueuedActionMarkerControlOffsets[] = {
    0x00, 0x5C, 0xD0
};

constexpr DWORD CenteredHudControlOffsets[] = {
    0x675C, 0x689C, 0x69DC,
    0x6CD0, 0x7058
};

constexpr DWORD ActionMenuControlOffsets[] = {
    0x00, 0x1C4, 0x388, 0x54C
};

struct CachedRect {
    char* control;
    Rect rect;
    bool valid;
};

constexpr int MaxCachedRects = 256;
CachedRect g_cachedRects[MaxCachedRects] = {};

constexpr DWORD CharacterControlOffsets[] = {
    0x28, 0x168, 0x2A8, 0x3E8, 0x528,
    0x668, 0x7A8, 0x8F8, 0xA48, 0xB88, 0xCC8
};

bool writeMemory(void* address, const void* replacement, size_t size) {
    __try {
        DWORD oldProtect = 0;
        if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }

        CopyMemory(address, replacement, size);

        DWORD ignored = 0;
        VirtualProtect(address, size, oldProtect, &ignored);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
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

bool safeWriteInt(void* address, int value) {
    return writeMemory(address, &value, sizeof(value));
}

bool safeReadRect(const void* address, Rect& value) {
    __try {
        value = *reinterpret_cast<const Rect*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = {};
        return false;
    }
}

int screenWidth() {
    int width = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenWidthAddress), width) || width <= 0) {
        return BaseWidth;
    }

    return width;
}

int screenHeight() {
    int height = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenHeightAddress), height) || height <= 0) {
        return BaseHeight;
    }

    return height;
}

int hudScale() {
    const int roundedScale = (screenHeight() + (BaseHeight / 2)) / BaseHeight;
    return roundedScale > 1 ? roundedScale : 1;
}

int hudCanvasLeft(int scale) {
    const int offset = (screenWidth() - (BaseWidth * scale)) / 2;
    return offset > 0 ? offset : 0;
}

bool hasUsefulRect(const Rect& rect) {
    return rect.width > 0 && rect.height > 0 &&
        rect.width < 4096 && rect.height < 4096;
}

bool isBaseRect(const Rect& rect) {
    return rect.left >= 0 &&
        rect.top >= 0 &&
        rect.left + rect.width <= BaseWidth &&
        rect.top + rect.height <= BaseHeight;
}

bool getBaseRectForControl(char* control, Rect& rect) {
    if (!control) {
        return false;
    }

    for (CachedRect& cached : g_cachedRects) {
        if (cached.valid && cached.control == control) {
            rect = cached.rect;
            return true;
        }
    }

    Rect current = {};
    if (!safeReadRect(control + 0x04, current) ||
        !hasUsefulRect(current) ||
        !isBaseRect(current)) {
        return false;
    }

    for (CachedRect& cached : g_cachedRects) {
        if (!cached.valid) {
            cached.control = control;
            cached.rect = current;
            cached.valid = true;
            rect = current;
            return true;
        }
    }

    rect = current;
    return true;
}

bool isMinimapControl(DWORD offset) {
    for (DWORD minimapOffset : MinimapOffsets) {
        if (offset == minimapOffset) {
            return true;
        }
    }

    return false;
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

    writeMemory(control + 0x04, &rect, sizeof(rect));
}

Rect scaleRect(const Rect& original, int scale) {
    Rect scaled = {
        original.left * scale,
        original.top * scale,
        original.width * scale,
        original.height * scale
    };

    if (original.left >= BaseWidth / 2) {
        const int rightGap = BaseWidth - (original.left + original.width);
        scaled.left = screenWidth() - (rightGap * scale) - scaled.width;
    }

    if (original.top >= BaseHeight / 2) {
        const int bottomGap = BaseHeight - (original.top + original.height);
        scaled.top = screenHeight() - (bottomGap * scale) - scaled.height;
    }

    return scaled;
}

Rect scaleQueuedActionMarkerRect(const Rect& original, int scale) {
    Rect scaled = {
        hudCanvasLeft(scale) + (original.left * scale),
        original.top * scale,
        original.width * scale,
        original.height * scale
    };

    if (original.top >= BaseHeight / 2) {
        const int bottomGap = BaseHeight - (original.top + original.height);
        scaled.top = screenHeight() - (bottomGap * scale) - scaled.height;
    }

    return scaled;
}

int scaleCoordinate(int value, int base, int target, int scale) {
    int scaled = value * scale;
    if (value >= base / 2) {
        scaled = target - ((base - value) * scale);
    }

    return scaled;
}

void scaleControl(char* base, DWORD offset, int scale) {
    if (!base || isMinimapControl(offset)) {
        return;
    }

    char* control = base + offset;
    Rect original = {};
    if (!getBaseRectForControl(control, original)) {
        return;
    }

    callControlSetRect(control, scaleRect(original, scale));
}

void scaleVolatileControl(char* base, DWORD offset, int scale) {
    if (!base) {
        return;
    }

    char* control = base + offset;
    Rect original = {};
    if (!safeReadRect(control + 0x04, original) || !hasUsefulRect(original)) {
        return;
    }

    Rect scaled = original;
    if (original.left >= 0 && original.left + original.width <= BaseWidth) {
        scaled.left = original.left * scale;
        scaled.width = original.width * scale;
        if (original.left >= BaseWidth / 2) {
            const int rightGap = BaseWidth - (original.left + original.width);
            scaled.left = screenWidth() - (rightGap * scale) - scaled.width;
        }
    }

    if (original.top >= 0 && original.top + original.height <= BaseHeight) {
        scaled.top = original.top * scale;
        scaled.height = original.height * scale;
        if (original.top >= BaseHeight / 2) {
            const int bottomGap = BaseHeight - (original.top + original.height);
            scaled.top = screenHeight() - (bottomGap * scale) - scaled.height;
        }
    }
    else if (original.top > BaseHeight && original.height <= BaseHeight / 4) {
        const int bottom = original.top + original.height;
        scaled.height = original.height * scale;
        scaled.top = bottom - scaled.height;
    }

    if (scaled.left != original.left ||
        scaled.top != original.top ||
        scaled.width != original.width ||
        scaled.height != original.height) {
        callControlSetRect(control, scaled);
    }
}

void scaleActionDescriptionBottom(char* base, int scale) {
    if (!base) {
        return;
    }

    int bottom = 0;
    if (!safeReadInt(base + ActionDescriptionBottomOffset, bottom) ||
        bottom <= 0 ||
        bottom > BaseHeight) {
        return;
    }

    const int scaled = scaleCoordinate(bottom, BaseHeight, screenHeight(), scale);
    if (scaled != bottom) {
        safeWriteInt(base + ActionDescriptionBottomOffset, scaled);
    }
}

void scaleActionDescriptionControls(char* base, int scale) {
    scaleActionDescriptionBottom(base, scale);
    for (DWORD offset : ActionDescriptionControlOffsets) {
        scaleVolatileControl(base, offset, scale);
    }
}

void scaleQueuedActionMarkerControl(char* control, int scale, bool useVirtualSetRect) {
    if (!control) {
        return;
    }

    Rect original = {};
    if (!getBaseRectForControl(control, original)) {
        return;
    }

    const Rect scaled = scaleQueuedActionMarkerRect(original, scale);
    if (useVirtualSetRect) {
        callControlSetRect(control, scaled);
    }
    else {
        writeMemory(control + 0x04, &scaled, sizeof(scaled));
    }
}

void scaleQueuedActionMarkers(char* base, int scale) {
    if (!base) {
        return;
    }

    for (DWORD markerOffset : QueuedActionMarkerOffsets) {
        char* marker = base + markerOffset;
        for (DWORD controlOffset : QueuedActionMarkerControlOffsets) {
            scaleQueuedActionMarkerControl(marker + controlOffset, scale, true);
        }

        int drawable = 0;
        if (safeReadInt(marker + 0xE4, drawable) && drawable != 0) {
            scaleQueuedActionMarkerControl(reinterpret_cast<char*>(drawable), scale, false);
        }
    }
}

void scaleCenteredHudControls(char* base, int scale) {
    if (!base) {
        return;
    }

    for (DWORD offset : CenteredHudControlOffsets) {
        char* marker = base + offset;
        for (DWORD controlOffset : QueuedActionMarkerControlOffsets) {
            scaleQueuedActionMarkerControl(marker + controlOffset, scale, true);
        }

        int drawable = 0;
        if (safeReadInt(marker + 0xE4, drawable) && drawable != 0) {
            scaleQueuedActionMarkerControl(reinterpret_cast<char*>(drawable), scale, false);
        }
    }
}

void scaleActionMenu(char* base, DWORD offset, int scale) {
    for (DWORD childOffset : ActionMenuControlOffsets) {
        scaleControl(base, offset + childOffset, scale);
    }
}

void scaleActionQueue(char* base, int scale) {
    constexpr DWORD ActionQueueBase = 0x772C;
    constexpr DWORD ActionQueueStride = 0x71C;
    for (int i = 0; i < 6; ++i) {
        scaleActionMenu(base, ActionQueueBase + (ActionQueueStride * i), scale);
    }
}

void scaleCharacter(char* base, DWORD offset, int scale) {
    for (DWORD childOffset : CharacterControlOffsets) {
        scaleControl(base, offset + childOffset, scale);
    }
}

void scaleStatusSummaries(char* base, int scale) {
    constexpr DWORD StatusBase = 0xA460;
    constexpr DWORD StatusStride = 0x14C;
    for (int i = 0; i < 9; ++i) {
        scaleControl(base, StatusBase + (StatusStride * i), scale);
    }
}

void scaleRootPanel(char* base) {
    Rect root = { 0, 0, screenWidth(), screenHeight() };
    callControlSetRect(base, root);
}

void scaleCenteredOwnerRect(char* owner, int scale) {
    if (!owner) {
        return;
    }

    Rect original = {};
    if (!safeReadRect(owner + 0x04, original) || !hasUsefulRect(original)) {
        return;
    }

    Rect scaled = original;
    if (isBaseRect(original)) {
        scaled = scaleQueuedActionMarkerRect(original, scale);
    }
    else {
        scaled.left = hudCanvasLeft(scale) + original.left;
    }

    if (scaled.left != original.left ||
        scaled.top != original.top ||
        scaled.width != original.width ||
        scaled.height != original.height) {
        writeMemory(owner + 0x04, &scaled, sizeof(scaled));
    }
}

}

void scaleHudControls(void* hud) {
    if (!hud) {
        return;
    }

    const int scale = hudScale();
    char* base = static_cast<char*>(hud);

    scaleActionDescriptionBottom(base, scale);

    scaleRootPanel(base);

    scaleActionMenu(base, 0x00BC, scale); // target_action_menu
    scaleActionQueue(base, scale);
    scaleQueuedActionMarkers(base, scale);
    scaleCenteredHudControls(base, scale);

    for (DWORD offset : HudControlOffsets) {
        scaleControl(base, offset, scale);
    }

    constexpr DWORD PartyBase = 0x1F88;
    constexpr DWORD PartyStride = 0xEA8;
    for (int i = 0; i < 3; ++i) {
        scaleCharacter(base, PartyBase + (PartyStride * i), scale);
    }
    scaleCharacter(base, 0x4B80, scale); // main_character

    scaleStatusSummaries(base, scale);
}

void scaleHudActionDescription(void* hud) {
    if (!hud || (screenWidth() == BaseWidth && screenHeight() == BaseHeight)) {
        return;
    }

    scaleActionDescriptionControls(static_cast<char*>(hud), hudScale());
}

void scaleAreaTransitionPrompt(void* prompt) {
    if (!prompt || (screenWidth() == BaseWidth && screenHeight() == BaseHeight)) {
        return;
    }

    scaleCenteredOwnerRect(static_cast<char*>(prompt), hudScale());
}

}
