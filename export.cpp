#include "widescreen_ui_scale.h"

using namespace WidescreenUiScale;

namespace {

bool g_quickChoiceDialogScale = false;

constexpr int QuickChoiceHeightBasis = 520;
constexpr int QuickChoiceCenterPadding = 8;
constexpr int QuickChoiceTopAnchor = 124;

bool isFourByThreeMenuRoot(const Rect& rect) {
    return rect.left == 0 &&
        rect.top == 0 &&
        rect.width >= 640 &&
        rect.height >= 480 &&
        rect.width * 3 == rect.height * 4;
}

bool isQuickChoiceDialogRootRect(const Rect& rect) {
    return
        rect.left >= 300 &&
        rect.left <= 430 &&
        rect.top >= 80 &&
        rect.top <= 190 &&
        rect.width >= 260 &&
        rect.width <= 360 &&
        rect.height >= 250 &&
        rect.height <= 360;
}

bool isQuickPanelRootRect(const Rect& rect) {
    const bool classStepPanel =
        rect.left == 403 &&
        rect.top == 109 &&
        rect.width == 334 &&
        rect.height == 344;

    return classStepPanel || isQuickChoiceDialogRootRect(rect);
}

void anchorQuickChoiceDialogRoot(Rect* rect) {
    if (!rect) {
        return;
    }

    const int viewportWidth = scaleCoordinate(BaseWidth, screenHeight(), BaseHeight);
    const int viewportLeft = (screenWidth() - viewportWidth) / 2;
    rect->left = (screenWidth() / 2) - viewportLeft + scaleCoordinate(QuickChoiceCenterPadding, screenHeight(), BaseHeight);
    rect->top = scaleCoordinate(QuickChoiceTopAnchor, screenHeight(), BaseHeight);
}

}

// Status-summary cached rect fix

extern "C" void __cdecl postLoadGuiControl(void* control) {
    if (!control) {
        return;
    }

    __try {
        Rect* rect = reinterpret_cast<Rect*>(static_cast<char*>(control) + 0x04);

        const bool statusSummaryChild =
            (rect->left == 10 && rect->top == 232 && rect->width == 32 && rect->height == 64) ||
            (rect->left == 10 && rect->top == 300 && rect->width == 32 && rect->height == 64) ||
            (rect->left == 52 && rect->top == 231 && rect->width == 260 && rect->height == 64) ||
            (rect->left == 52 && rect->top == 300 && rect->width == 260 && rect->height == 64) ||
            (rect->left == 156 && rect->top == 40 && rect->width == 100 && rect->height == 44) ||
            (rect->left == 114 && rect->top == 364 && rect->width == 100 && rect->height == 44);

        if (!statusSummaryChild) {
            return;
        }

        copyStatusSummaryCachedRects(control, *rect);
        scaleStatusSummaryParentFrame(control, *rect);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// Status-summary popup layout fix

extern "C" void __cdecl postStatusSummaryLayout(void* statusSummary) {
    if (!statusSummary) {
        return;
    }

    __try {
        Rect* root = reinterpret_cast<Rect*>(static_cast<char*>(statusSummary) + 0x04);
        void* okControl = static_cast<char*>(statusSummary) + 0x1980;
        Rect* okButton = reinterpret_cast<Rect*>(static_cast<char*>(okControl) + 0x04);
        const Rect beforeRoot = *root;
        const Rect beforeOk = *okButton;
        char* base = static_cast<char*>(statusSummary);
        int firstActive = -1;
        int contentBottom = 0;

        for (int i = 0; i < 9; ++i) {
            Rect* icon = reinterpret_cast<Rect*>(base + 0x300 + i * 0x140 + 0x04);
            Rect* text = reinterpret_cast<Rect*>(base + 0xE40 + i * 0x140 + 0x04);
            DWORD iconFlags = 0;
            DWORD textFlags = 0;
            safeReadDword(base + 0x300 + i * 0x140 + 0x44, iconFlags);
            safeReadDword(base + 0xE40 + i * 0x140 + 0x44, textFlags);

            if ((iconFlags & 2) != 0 || (textFlags & 2) != 0) {
                if (firstActive < 0) {
                    firstActive = i;
                }
            }

            if ((iconFlags & 2) != 0 && contentBottom < icon->top + icon->height) {
                contentBottom = icon->top + icon->height;
            }

            if ((textFlags & 2) != 0 && contentBottom < text->top + text->height) {
                contentBottom = text->top + text->height;
            }
        }

        if (firstActive < 0 || root->width <= 0 || root->height <= 0) {
            return;
        }

        Rect scaledRoot = beforeRoot;
        const int height = screenHeight();
        int scaledGap = StatusSummaryTextButtonGap;
        if (height > BaseHeight) {
            scaledGap = scaleCoordinate(StatusSummaryTextButtonGap, height, BaseHeight);
            Rect shiftedOk = beforeOk;
            shiftedOk.top = contentBottom + scaledGap;
            setControlRect(okControl, shiftedOk);

            const int okDelta = shiftedOk.top - beforeOk.top;
            scaledRoot.height = scaleCoordinate(beforeRoot.height, height, BaseHeight) + okDelta;
            if (scaledRoot.height > beforeRoot.height) {
                scaledRoot.top = beforeRoot.top - ((scaledRoot.height - beforeRoot.height) / 2);
                setControlRect(statusSummary, scaledRoot);
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// Fullscreen map buffer fix

extern "C" void __cdecl prepareAreaMapScale(void* areaMap) {
    UNREFERENCED_PARAMETER(areaMap);

    __try {
        WidescreenUiScale::prepareAreaMapScale();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareAreaMapDimensionsForScreen() {
    __try {
        if (!g_hudScale) {
            WidescreenUiScale::prepareAreaMapDimensionsForScreen();
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareHudMinimapMapCoordinates(void* areaMap) {
    __try {
        WidescreenUiScale::prepareHudMinimapMapCoordinates(areaMap);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareHudMinimapRenderCoordinates(void* areaMap) {
    UNREFERENCED_PARAMETER(areaMap);

    __try {
        WidescreenUiScale::prepareHudMinimapRenderCoordinates();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl adjustHudMinimapCenterCoordinates(void* hud, int* mapX, int* mapY) {
    __try {
        WidescreenUiScale::adjustHudMinimapCenterCoordinates(hud, mapX, mapY);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl adjustHudMinimapFogGridDimensions(void* hud, int* mapWidth, int* mapHeight) {
    __try {
        WidescreenUiScale::adjustHudMinimapFogGridDimensions(hud, mapWidth, mapHeight);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// GUI extent scaling fix

extern "C" void __cdecl scaleGuiExtent(Rect* rect, DWORD eax, DWORD ecx, DWORD edx, DWORD* stack) {
    if (!rect) {
        return;
    }

    UNREFERENCED_PARAMETER(eax);
    UNREFERENCED_PARAMETER(ecx);

    __try {
        if (isBaseResolution()) {
            return;
        }

        if (!g_areaMapDimensionsPrepared && screenWidth() >= 640 && screenHeight() >= 480) {
            WidescreenUiScale::prepareAreaMapDimensionsForScreen();
        }

        bool scaleCurrentRect = true;
        const Rect originalRect = *rect;

        if (rect->width > 0 && rect->height > 0) {
            const bool rootLoad = isRootLoadContext(stack);
            const bool topTabRoot = isTopTabRoot(*rect, stack);
            const bool menuRoot = rootLoad;
            const bool classSelectionRoot = rootLoad && isClassSelectionRootContext(stack);
            const bool hudRoot = rootLoad && isHudRootContext(stack);
            const bool containerRoot = rootLoad && isContainerRootContext(stack);
            const bool quickChoiceDialogRoot = rootLoad && isQuickChoiceDialogRootRect(*rect);
            const bool quickPanelRoot = rootLoad && (isQuickPanelRootContext(stack) || isQuickPanelRootRect(*rect));
            const bool popupRoot = rootLoad && isPopupRootContext(stack);
            const bool fadeRoot = rootLoad && isFadeRootContext(stack);

            if (rootLoad) {
                g_quickChoiceDialogScale = false;
                resetAreaMapOverlayTransform();
                if (callContextContains(stack, "galaxymap") ||
                    memoryContainsText(reinterpret_cast<const void*>(edx), 512, "galaxymap")) {
                    activateAreaMapOverlayTransformForScreen();
                }
            }

            if (topTabRoot && menuRoot) {
                setScale(rect->width, 480, 640, 480, false);
            }
            else if (hudRoot && rect->width == 800 && rect->height == 600) {
                setHudScale(rect->width, rect->height);
                scaleCurrentRect = false;
            }
            else if (containerRoot) {
                setScale(640, 480, 640, 480, true);
            }
            else if (quickPanelRoot) {
                if (quickChoiceDialogRoot) {
                    g_quickChoiceDialogScale = true;
                    setHeightBasisScale(BaseWidth, BaseHeight, QuickChoiceHeightBasis, false);
                }
                else {
                    setScreenScale(800, 600, true);
                }
            }
            else if (popupRoot) {
                setScale(640, 480, 640, 480, true);
            }
            else if (fadeRoot) {
                setScale(640, 480, 640, 480, true);
                scaleCurrentRect = false;
            }
            else if (menuRoot && isFourByThreeMenuRoot(*rect)) {
                setScale(rect->width, rect->height, rect->width, rect->height, true);
                g_classSelectionScale = classSelectionRoot;
            }
            else if (rootLoad && rect->left == 0 && rect->top == 0) {
                setIdentityScale(rect->width, rect->height, true);
            }
            else if (rootLoad) {
                setIdentityScale(rect->width, rect->height, false);
            }
        }

        if (!scaleCurrentRect) {
            return;
        }

        if (g_hudScale) {
            if (applyHudMinimapIntegerScale(rect, stack)) {
                return;
            }

            rect->left = scaleCoordinate(rect->left, g_guiTargetHeight, g_guiBaseHeight);
            rect->width = scaleCoordinate(rect->width, g_guiTargetHeight, g_guiBaseHeight);
            rect->top = scaleCoordinate(rect->top, g_guiTargetHeight, g_guiBaseHeight);
            rect->height = scaleCoordinate(rect->height, g_guiTargetHeight, g_guiBaseHeight);
            if (isHudRightAnchored(originalRect)) {
                rect->left += hudRightOffset();
            }
        }
        else {
            if (isMenuMapViewportControl(originalRect, stack)) {
                anchorUnscaledMenuMapViewport(rect);
            }
            else if (!applyAreaMapOverlayTransform(rect, stack)) {
                if (g_classSelectionScale && isClassSelectionPickerRect(originalRect)) {
                    applyGuiExtentEdit(rect);
                }
                else {
                    rect->left = scaleCoordinate(rect->left, g_guiTargetWidth, g_guiBaseWidth);
                    rect->width = scaleCoordinate(rect->width, g_guiTargetWidth, g_guiBaseWidth);
                    rect->top = scaleCoordinate(rect->top, g_guiTargetHeight, g_guiBaseHeight);
                    rect->height = scaleCoordinate(rect->height, g_guiTargetHeight, g_guiBaseHeight);
                    if (g_quickChoiceDialogScale && isQuickChoiceDialogRootRect(originalRect)) {
                        anchorQuickChoiceDialogRoot(rect);
                    }
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// DLL lifecycle

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reserved);

    if (reason == DLL_PROCESS_ATTACH) {
        g_classSelectionScale = false;
        g_areaMapDimensionsPrepared = false;
        resetAreaMapOverlayTransform();
        __try {
            if (screenWidth() >= 640 && screenHeight() >= 480) {
                WidescreenUiScale::prepareAreaMapDimensionsForScreen();
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    return TRUE;
}
