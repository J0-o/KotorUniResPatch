#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Root-load detection helpers

namespace {

bool isGuiLoadReturn(DWORD* stack) {
    if (!stack) {
        return false;
    }

    DWORD returnAddress = 0;
    return safeReadDword(stack + 2, returnAddress) && returnAddress == 0x0040A713;
}

bool loadContextContains(DWORD* stack, const char* token) {
    if (!stack) {
        return false;
    }

    DWORD savedEdi = 0;
    DWORD firstLoadArg = 0;
    DWORD secondLoadArg = 0;
    safeReadDword(stack + 0, savedEdi);
    safeReadDword(stack + 3, firstLoadArg);
    safeReadDword(stack + 4, secondLoadArg);

    return gffContextContains(savedEdi, token) ||
        gffContextContains(firstLoadArg, token) ||
        gffContextContains(secondLoadArg, token);
}

}

bool isClassSelectionRootContext(DWORD* stack) {
    return isGuiLoadReturn(stack) && loadContextContains(stack, "classsel");
}

bool isHudRootContext(DWORD* stack) {
    if (!isGuiLoadReturn(stack)) {
        return false;
    }

    return loadContextContains(stack, "maininterface") ||
        loadContextContains(stack, "mi8x6") ||
        loadContextContains(stack, "mipc");
}

bool isContainerRootContext(DWORD* stack) {
    if (!isGuiLoadReturn(stack)) {
        return false;
    }

    return loadContextContains(stack, "container");
}

bool isQuickPanelRootContext(DWORD* stack) {
    return isGuiLoadReturn(stack) && loadContextContains(stack, "quickpnl");
}

bool isPopupRootContext(DWORD* stack) {
    if (!isGuiLoadReturn(stack)) {
        return false;
    }

    return loadContextContains(stack, "confirm") ||
        loadContextContains(stack, "tooltip") ||
        loadContextContains(stack, "barkbubble") ||
        loadContextContains(stack, "dialog") ||
        loadContextContains(stack, "debug") ||
        loadContextContains(stack, "pause") ||
        loadContextContains(stack, "areatransition") ||
        loadContextContains(stack, "optresolution") ||
        loadContextContains(stack, "leveluppnl") ||
        loadContextContains(stack, "pwrlvlup") ||
        loadContextContains(stack, "statussummary");
}

bool isFadeRootContext(DWORD* stack) {
    return isGuiLoadReturn(stack) && loadContextContains(stack, "fade");
}

bool isRootLoadContext(DWORD* stack) {
    return isGuiLoadReturn(stack);
}

bool isTopTabRoot(const Rect& rect, DWORD* stack) {
    return isRootLoadContext(stack) &&
        rect.left == 0 &&
        rect.top == 0 &&
        rect.width == 640 &&
        rect.height == 86;
}


}
