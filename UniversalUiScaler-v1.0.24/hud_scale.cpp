#include "hud_scale.h"

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
    0x5CC0, // Minimap border.
    0x5E00, // Minimap image.
    0x5F40, // Player arrow.
    0x6098  // Minimap button.
};

constexpr DWORD HudControlOffsets[] = {
    0x1BC8, 0x1D08, 0x1E48,             // Party frame trim.
    0x675C, 0x689C, 0x69DC,             // Combat background panels.
    0x6CD0, 0x6E94, 0x7058,             // Clear action buttons.
    0x735C, 0x749C,                     // Combat mode text.
    0xA1D4, 0xA314,                     // Action description text.
    0xB00C, 0xB1D0, 0xB394, 0xB558,     // Main menu buttons.
    0xB71C, 0xB8E0, 0xBAA4, 0xBC68,
    0xBE2C, 0xBF6C,                     // Menu background and arrow margin.
    0xC0AC, 0xC360, 0xC614              // HUD toggle buttons.
};

constexpr DWORD ActionMenuControlOffsets[] = {
    0x00, 0x1C4, 0x388, 0x54C
};

void* g_scaledHud = nullptr;
int g_scaledWidth = 0;
int g_scaledHeight = 0;

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

bool hasUsefulRect(const Rect& rect) {
    return rect.width > 0 && rect.height > 0 &&
        rect.width < 4096 && rect.height < 4096;
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

void scaleControl(char* base, DWORD offset, int scale) {
    if (!base || isMinimapControl(offset)) {
        return;
    }

    char* control = base + offset;
    Rect original = {};
    if (!safeReadRect(control + 0x04, original) || !hasUsefulRect(original)) {
        return;
    }

    callControlSetRect(control, scaleRect(original, scale));
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

}

void scaleHudControls(void* hud) {
    if (!hud) {
        return;
    }

    const int width = screenWidth();
    const int height = screenHeight();
    if (hud == g_scaledHud && width == g_scaledWidth && height == g_scaledHeight) {
        return;
    }

    const int scale = hudScale();

    char* base = static_cast<char*>(hud);
    scaleRootPanel(base);

    scaleActionMenu(base, 0x00BC, scale); // Target action menu.
    scaleActionQueue(base, scale);

    for (DWORD offset : HudControlOffsets) {
        scaleControl(base, offset, scale);
    }

    constexpr DWORD PartyBase = 0x1F88;
    constexpr DWORD PartyStride = 0xEA8;
    for (int i = 0; i < 3; ++i) {
        scaleCharacter(base, PartyBase + (PartyStride * i), scale);
    }
    scaleCharacter(base, 0x4B80, scale); // Main character panel.

    scaleStatusSummaries(base, scale);

    g_scaledHud = hud;
    g_scaledWidth = width;
    g_scaledHeight = height;
}

}
