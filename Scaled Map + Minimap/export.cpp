#include "scaled_map.h"
#include "scaled_minimap.h"

namespace {

constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;

bool isBaseResolution() {
    return *reinterpret_cast<int*>(ScreenWidthAddress) == 800 &&
        *reinterpret_cast<int*>(ScreenHeightAddress) == 600;
}

}

extern "C" void __cdecl prepareMenuMapScale(void* map) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        WidescreenUiScale::prepareMenuMapScale(map);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapIconMaterials() {
    __try {
        if (isBaseResolution()) {
            return;
        }
        WidescreenUiScale::prepareMenuMapIconMaterials();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapDraw(void* map, int* width) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        WidescreenUiScale::prepareMenuMapDraw(map, width);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareMenuMapMarkerDraw(WidescreenUiScale::Rect* rect) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        WidescreenUiScale::prepareMenuMapMarkerDraw(rect);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareAreaMapDimensionsForScreen() {
    __try {
        if (isBaseResolution()) {
            return;
        }
        WidescreenUiScale::prepareAreaMapDimensionsForScreen();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleHudMinimapExtent(HudMinimapScale::Rect* rect, DWORD* stack) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        HudMinimapScale::scaleHudMinimapExtent(rect, stack);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl prepareHudMinimapScale(void* hud, int* mapX, int* mapY, int* rectWidth, int* rectHeight) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        HudMinimapScale::prepareHudMinimapScale(hud, mapX, mapY, rectWidth, rectHeight);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl zoomHudMinimapImageDraw(void* image, int* x, int* y, int* width, int* height) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        HudMinimapScale::zoomHudMinimapImageDraw(image, x, y, width, height);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl beginHudMinimapGridZoom(void* hud, HudMinimapScale::Rect* rect) {
    __try {
        if (isBaseResolution()) {
            return;
        }
        HudMinimapScale::beginHudMinimapGridZoom(hud, rect);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl endHudMinimapGridZoom(void* hud) {
    __try {
        if (isBaseResolution()) {
            return;
        }
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
