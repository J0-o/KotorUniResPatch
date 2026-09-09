#include "scaled_map.h"
#include "scaled_minimap.h"

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

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(reserved);
    return TRUE;
}
