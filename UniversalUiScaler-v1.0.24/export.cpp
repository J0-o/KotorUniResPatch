#include "area_hud_minimap_scale.h"
#include "area_map_scale.h"
#include "class_selection_layout_constants.h"
#include "container_popup_scale.h"
#include "hud_scale.h"
#include "levelupbox_scale.h"
#include "menu_scale.h"
#include "popup_dialog_scale.h"

extern "C" void __cdecl scaleMenuPanelTree(void* panel) {
    __try {
        MenuScale::scaleMenuPanelTree(panel);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scalePazaakGameCards(void* pazaakGame) {
    __try {
        MenuScale::scalePazaakGameCards(pazaakGame);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleQuickOrCustomPanel(void* owner) {
    __try {
        LevelUpBoxScale::scaleQuickOrCustomPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleQuickPanel(void* owner) {
    __try {
        LevelUpBoxScale::scaleQuickPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleLevelUpPanel(void* owner) {
    __try {
        LevelUpBoxScale::scaleLevelUpPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleCustomPanel(void* owner) {
    __try {
        LevelUpBoxScale::scaleCustomPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl patchClassSelectionLayoutRects(void* ownerStackSlot, void* currentSlotMarker, void* baseRectStack, void* wrapperRectStack) {
    __try {
        ClassSelectionLayoutConstants::patchClassSelectionLayoutRects(ownerStackSlot, currentSlotMarker, baseRectStack, wrapperRectStack);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl patchInitialClassSelectionRects(void* owner) {
    __try {
        ClassSelectionLayoutConstants::patchInitialClassSelectionRects(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scalePopupDialogPanel(void* owner) {
    __try {
        PopupDialogScale::scalePopupDialogPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleStatusSummarySetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    __try {
        PopupDialogScale::scaleStatusSummarySetRect(control, returnAddressSlot, rectPointerSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleMessageBoxButtonSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    __try {
        PopupDialogScale::scaleMessageBoxButtonSetRect(control, returnAddressSlot, rectPointerSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleMessageBoxLabelSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    __try {
        PopupDialogScale::scaleMessageBoxLabelSetRect(control, returnAddressSlot, rectPointerSlot);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleMessageBoxAfterFix(void* owner) {
    __try {
        PopupDialogScale::scaleMessageBoxAfterFix(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleContainerPanel(void* owner) {
    __try {
        ContainerPopupScale::scaleContainerPanel(owner);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapScale(void* map) {
    __try {
        WidescreenUiScale::prepareMenuMapScale(map);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapIconMaterials() {
    __try {
        WidescreenUiScale::prepareMenuMapIconMaterials();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapDraw(void* map, int* width) {
    __try {
        WidescreenUiScale::prepareMenuMapDraw(map, width);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapMarkerDraw(WidescreenUiScale::Rect* rect) {
    __try {
        WidescreenUiScale::prepareMenuMapMarkerDraw(rect);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareAreaMapDimensionsForScreen() {
    __try {
        WidescreenUiScale::prepareAreaMapDimensionsForScreen();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleHudMinimapExtent(HudMinimapScale::Rect* rect, DWORD* stack) {
    __try {
        HudMinimapScale::scaleHudMinimapExtent(rect, stack);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareHudMinimapScale(void* hud, int* mapX, int* mapY, int* rectWidth, int* rectHeight) {
    __try {
        HudMinimapScale::prepareHudMinimapScale(hud, mapX, mapY, rectWidth, rectHeight);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl zoomHudMinimapImageDraw(void* image, int* x, int* y, int* width, int* height) {
    __try {
        HudMinimapScale::zoomHudMinimapImageDraw(image, x, y, width, height);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl beginHudMinimapGridZoom(void* hud, HudMinimapScale::Rect* rect) {
    __try {
        HudMinimapScale::beginHudMinimapGridZoom(hud, rect);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl endHudMinimapGridZoom(void* hud) {
    __try {
        HudMinimapScale::endHudMinimapGridZoom(hud);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleHudControls(void* hud) {
    __try {
        HudScale::scaleHudControls(hud);
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
