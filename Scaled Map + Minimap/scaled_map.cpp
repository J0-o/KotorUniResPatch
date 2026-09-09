#include "scaled_map.h"
#include "../Common/ResolutionScale.h"

namespace WidescreenUiScale {

namespace {

constexpr int UiBaseHeight = 480;
constexpr int AreaMapTextureWidth = 512;
constexpr int AreaMapTextureHeight = 256;
constexpr int AreaMapViewportWidth = 440;
constexpr int AreaMapViewportHeight = 256;
constexpr int ArrowIconSize = 0x20;
constexpr int CircleIconSize = 0x10;
constexpr int TargetIconSize = 0x14;
constexpr DWORD MapViewOffset = 0x0064;
constexpr DWORD MapHiderOffset = 0x0E38;
constexpr DWORD MapTextureOffset = 0x1080;

Rect makeRect(int left, int top, int width, int height) {
    return { left, top, width, height };
}

bool writeMemory(void* address, const void* replacement, size_t size) {
    __try {
        DWORD oldProtect = 0;
        if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }

        CopyMemory(address, replacement, size);
        FlushInstructionCache(GetCurrentProcess(), address, size);

        DWORD ignored = 0;
        VirtualProtect(address, size, oldProtect, &ignored);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void writeInt(int address, int value) {
    writeMemory(reinterpret_cast<void*>(address), &value, sizeof(value));
}

void writeFloat(int address, float value) {
    writeMemory(reinterpret_cast<void*>(address), &value, sizeof(value));
}

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

int scaleMapValue(int value, const UniversalScaleState& scale) {
    return scaleUiValueFromBase(value, BaseHeight, UiBaseHeight, scale);
}

void setControlSize(char* control, int width, int height) {
    if (!control) {
        return;
    }

    writeMemory(control + 0x0C, &width, sizeof(width));
    writeMemory(control + 0x10, &height, sizeof(height));
}

void setControlRect(char* control, const Rect& rect) {
    if (!control) {
        return;
    }

    __try {
        DWORD vtable = 0;
        DWORD setRect = 0;
        if (safeReadDword(control, vtable) &&
            safeReadDword(reinterpret_cast<const void*>(vtable + 4), setRect) &&
            setRect != 0) {
            typedef void(__thiscall *SetRectFn)(void*, const Rect*);
            reinterpret_cast<SetRectFn>(setRect)(control, &rect);
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    writeMemory(control + 0x04, &rect, sizeof(rect));
}

int scaledMarkerDimension(int value, const UniversalScaleState& scale) {
    return value > 0 ? scaleMapValue(value, scale) : value;
}

void patchAreaMapCoordinateBounds(const UniversalScaleState& scale) {
    const int viewportWidth = scaleMapValue(AreaMapViewportWidth, scale);
    const int viewportHeight = scaleMapValue(AreaMapViewportHeight, scale);

    writeInt(0x00579009, viewportWidth);
    writeInt(0x0057901A, viewportHeight);
    writeInt(0x00578E9B, viewportWidth);
    writeInt(0x00578EA6, viewportHeight);
    writeInt(0x00578F15, viewportWidth);
    writeInt(0x00578F24, viewportHeight);
    writeInt(0x00579344, viewportWidth);
    writeInt(0x00579358, viewportHeight);
    writeInt(0x00579377, viewportWidth);
    writeInt(0x0057937E, viewportWidth);
    writeInt(0x00579383, viewportHeight);
    writeInt(0x0057938A, viewportHeight);

    writeFloat(0x00747748, static_cast<float>(viewportWidth));
    writeFloat(0x007455D4, static_cast<float>(viewportHeight));
}

void patchAreaMapIconMaterialSizes(const UniversalScaleState& scale) {
    writeInt(0x0069405B, scaleMapValue(ArrowIconSize, scale));
    writeInt(0x006940DC, scaleMapValue(CircleIconSize, scale));
    writeInt(0x0069418F, scaleMapValue(TargetIconSize, scale));
}

void patchAreaMapEngineDimensions(const UniversalScaleState& scale) {
    writeInt(0x0069505C, scaleMapValue(AreaMapTextureWidth, scale));
    writeInt(0x00695064, scaleMapValue(AreaMapTextureHeight, scale));
    writeInt(0x00695082, scaleMapValue(AreaMapViewportWidth, scale));
    writeInt(0x0069508A, scaleMapValue(AreaMapViewportHeight, scale));

    patchAreaMapCoordinateBounds(scale);
    patchAreaMapIconMaterialSizes(scale);
}

void scaleLiveMenuMapControlSizes(void* map, const UniversalScaleState& scale) {
    if (!map) {
        return;
    }

    char* base = static_cast<char*>(map);
    char* mapView = base + MapViewOffset;
    char* mapHider = base + MapHiderOffset;
    char* mapTexture = base + MapTextureOffset;
    const int viewportWidth = scaleMapValue(AreaMapViewportWidth, scale);
    const int viewportHeight = scaleMapValue(AreaMapViewportHeight, scale);

    Rect mapViewRect = {};
    if (safeReadRect(mapView + 0x04, mapViewRect)) {
        mapViewRect.left -= (viewportWidth - mapViewRect.width) / 2;
        mapViewRect.top -= (viewportHeight - mapViewRect.height) / 2;
        mapViewRect.width = viewportWidth;
        mapViewRect.height = viewportHeight;
        setControlRect(mapView, mapViewRect);
    }
    else {
        setControlSize(mapView, viewportWidth, viewportHeight);
    }

    setControlRect(mapHider, makeRect(0, 0, viewportWidth, viewportHeight));
    setControlRect(mapTexture, makeRect(
        0, 0,
        scaleMapValue(AreaMapTextureWidth, scale),
        scaleMapValue(AreaMapTextureHeight, scale)));
}

}

void prepareMenuMapScale(void* map) {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale || (scale->uiWidth == BaseWidth && scale->uiHeight == BaseHeight)) {
        return;
    }

    patchAreaMapEngineDimensions(*scale);
    scaleLiveMenuMapControlSizes(map, *scale);
}

void prepareMenuMapIconMaterials() {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale || (scale->uiWidth == BaseWidth && scale->uiHeight == BaseHeight)) {
        return;
    }

    patchAreaMapIconMaterialSizes(*scale);
}

void prepareMenuMapDraw(void* map, int* width) {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale || (scale->uiWidth == BaseWidth && scale->uiHeight == BaseHeight)) {
        return;
    }

    patchAreaMapEngineDimensions(*scale);
    scaleLiveMenuMapControlSizes(map, *scale);

    if (width) {
        *width = scaleMapValue(AreaMapViewportWidth, *scale);
    }
}

void prepareMenuMapMarkerDraw(Rect* rect) {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!rect ||
        !scale ||
        (scale->uiWidth == BaseWidth && scale->uiHeight == BaseHeight) ||
        rect->width <= 0 ||
        rect->height <= 0 ||
        rect->width > 64 ||
        rect->height > 64) {
        return;
    }

    const int scaledWidth = scaledMarkerDimension(rect->width, *scale);
    const int scaledHeight = scaledMarkerDimension(rect->height, *scale);
    rect->left -= (scaledWidth - rect->width) / 2;
    rect->top -= (scaledHeight - rect->height) / 2;
    rect->width = scaledWidth;
    rect->height = scaledHeight;
}

void prepareAreaMapDimensionsForScreen() {
    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale || (scale->uiWidth == BaseWidth && scale->uiHeight == BaseHeight)) {
        return;
    }

    patchAreaMapEngineDimensions(*scale);
}

}
