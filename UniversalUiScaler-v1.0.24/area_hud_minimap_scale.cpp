#include "area_hud_minimap_scale.h"

namespace HudMinimapScale {

namespace {

constexpr int BaseHeight = 600;
constexpr float AreaMapViewportWidth = 440.0f;
constexpr float AreaMapViewportHeight = 256.0f;
constexpr DWORD AreaMapViewportWidthAddress = 0x00747748;
constexpr DWORD AreaMapViewportHeightAddress = 0x007455D4;
constexpr int MinimapViewportSize = 120;
constexpr int MinimapTextureSize = 512;
constexpr int MinimapAtlasHalfHeight = 256;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;
constexpr DWORD ViewportIndexAddress = 0x007B9460;
constexpr DWORD ViewportWidthAddress = 0x007B946C;
constexpr DWORD ViewportHeightAddress = 0x007B946E;
constexpr int ViewportEntryStride = 10;
constexpr int BorderLeft = -2;
constexpr int BorderTop = -3;
constexpr int BorderWidth = 136;
constexpr int BorderHeight = 137;
constexpr int MapViewLeft = 6;
constexpr int MapViewTop = 6;
constexpr int FogViewLeft = 8;
constexpr int FogViewTop = 8;
constexpr int FogViewSize = 118;
constexpr int ArrowLeft = 47;
constexpr int ArrowTop = 49;
constexpr int ArrowSize = 32;

constexpr Rect FloatingTargetActionRects[] = {
    { 43, 35, 35, 59 },
    { 45, 49, 32, 32 },
    { 43, 36, 35, 12 },
    { 44, 80, 35, 12 },
    { 83, 35, 35, 59 },
    { 85, 49, 32, 32 },
    { 83, 36, 35, 12 },
    { 84, 80, 35, 12 },
    { 122, 35, 35, 59 },
    { 124, 49, 32, 32 },
    { 122, 36, 35, 12 },
    { 123, 80, 35, 12 },
    { 0, 0, 200, 26 },
    { 0, 27, 200, 6 },
    { 6, 465, 35, 35 }
};

bool g_drawActive = false;
float g_areaMapViewportWidthBeforeGrid = AreaMapViewportWidth;
float g_areaMapViewportHeightBeforeGrid = AreaMapViewportHeight;
bool g_haveAreaMapViewportBeforeGrid = false;

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

bool safeReadShort(const void* address, short& value) {
    __try {
        value = *reinterpret_cast<const short*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0;
        return false;
    }
}

bool safeReadFloat(const void* address, float& value) {
    __try {
        value = *reinterpret_cast<const float*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0.0f;
        return false;
    }
}

void restoreAreaMapViewportAfterGrid() {
    if (!g_haveAreaMapViewportBeforeGrid) {
        return;
    }

    writeMemory(reinterpret_cast<void*>(AreaMapViewportWidthAddress),
        &g_areaMapViewportWidthBeforeGrid,
        sizeof(g_areaMapViewportWidthBeforeGrid));
    writeMemory(reinterpret_cast<void*>(AreaMapViewportHeightAddress),
        &g_areaMapViewportHeightBeforeGrid,
        sizeof(g_areaMapViewportHeightBeforeGrid));
    g_haveAreaMapViewportBeforeGrid = false;
}

int screenHeight() {
    int height = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenHeightAddress), height) || height <= 0) {
        return BaseHeight;
    }

    return height;
}

int minimapScale() {
    const int roundedScale = (screenHeight() + (BaseHeight / 2)) / BaseHeight;
    return roundedScale > 1 ? roundedScale : 1;
}

void callControlSetRect(char* control, const Rect& rect) {
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

void setControlRect(char* control, const Rect& rect) {
    if (!control) {
        return;
    }

    callControlSetRect(control, rect);
}

bool isRect(const Rect& rect, int left, int top, int width, int height) {
    return rect.left == left &&
        rect.top == top &&
        rect.width == width &&
        rect.height == height;
}

bool scaleKnownHudMinimapRect(Rect* rect) {
    const Rect original = *rect;
    const int scale = minimapScale();

    if (isRect(original, BorderLeft, BorderTop, BorderWidth, BorderHeight)) {
        rect->left = BorderLeft * scale;
        rect->top = BorderTop * scale;
        rect->width = BorderWidth * scale;
        rect->height = BorderHeight * scale;
        return true;
    }

    if (isRect(original, MapViewLeft, MapViewTop, MinimapTextureSize, MinimapTextureSize)) {
        rect->left = MapViewLeft * scale;
        rect->top = MapViewTop * scale;
        rect->width = MinimapTextureSize;
        rect->height = MinimapTextureSize;
        return true;
    }

    if (isRect(original, MapViewLeft, MapViewTop, MinimapViewportSize, MinimapViewportSize)) {
        rect->left = MapViewLeft * scale;
        rect->top = MapViewTop * scale;
        rect->width = MinimapViewportSize * scale;
        rect->height = MinimapViewportSize * scale;
        return true;
    }

    if (isRect(original, FogViewLeft, FogViewTop, FogViewSize, FogViewSize)) {
        rect->left = FogViewLeft * scale;
        rect->top = FogViewTop * scale;
        rect->width = FogViewSize * scale;
        rect->height = FogViewSize * scale;
        return true;
    }

    if (isRect(original, ArrowLeft, ArrowTop, ArrowSize, ArrowSize)) {
        rect->left = ArrowLeft * scale;
        rect->top = ArrowTop * scale;
        rect->width = ArrowSize * scale;
        rect->height = ArrowSize * scale;
        return true;
    }

    return false;
}

int scaleFrom800x600Height(int value) {
    return static_cast<int>((static_cast<long long>(value) * screenHeight()) / BaseHeight);
}

bool scaleFloatingTargetActionRect(Rect* rect) {
    const Rect original = *rect;

    for (const Rect& targetRect : FloatingTargetActionRects) {
        if (isRect(original, targetRect.left, targetRect.top, targetRect.width, targetRect.height)) {
            rect->left = scaleFrom800x600Height(rect->left);
            rect->top = scaleFrom800x600Height(rect->top);
            rect->width = scaleFrom800x600Height(rect->width);
            rect->height = scaleFrom800x600Height(rect->height);
            return true;
        }
    }

    return false;
}

bool isMinimapViewportActive() {
    DWORD viewportIndex = 0;
    if (!safeReadDword(reinterpret_cast<const void*>(ViewportIndexAddress), viewportIndex) ||
        viewportIndex > 31) {
        return false;
    }

    const DWORD offset = viewportIndex * ViewportEntryStride;
    short width = 0;
    short height = 0;
    if (!safeReadShort(reinterpret_cast<const void*>(ViewportWidthAddress + offset), width) ||
        !safeReadShort(reinterpret_cast<const void*>(ViewportHeightAddress + offset), height)) {
        return false;
    }

    const int viewportSize = MinimapViewportSize * minimapScale();
    return width == viewportSize && height == viewportSize;
}

}

void scaleHudMinimapExtent(Rect* rect, DWORD* stack) {
    UNREFERENCED_PARAMETER(stack);

    if (!rect) {
        return;
    }

    if (scaleKnownHudMinimapRect(rect)) {
        return;
    }

    scaleFloatingTargetActionRect(rect);
}

void prepareHudMinimapScale(void* hud, int* mapX, int* mapY, int* rectWidth, int* rectHeight) {
    UNREFERENCED_PARAMETER(rectWidth);
    UNREFERENCED_PARAMETER(rectHeight);

    if (!hud) {
        return;
    }

    g_drawActive = true;

    const int scale = minimapScale();
    if (scale > 1) {
        if (mapX) {
            *mapX /= scale;
        }
        if (mapY) {
            *mapY /= scale;
        }
    }

    const int viewportSize = MinimapViewportSize * scale;
    char* base = static_cast<char*>(hud);
    writeMemory(base + 0x6088, &viewportSize, sizeof(viewportSize));
    writeMemory(base + 0x608C, &viewportSize, sizeof(viewportSize));

    Rect borderRect = {
        BorderLeft * scale,
        BorderTop * scale,
        BorderWidth * scale,
        BorderHeight * scale
    };
    setControlRect(base + 0x5CC0, borderRect);

    Rect arrowRect = {
        (viewportSize - (ArrowSize * scale)) / 2,
        (viewportSize - (ArrowSize * scale)) / 2,
        ArrowSize * scale,
        ArrowSize * scale
    };
    setControlRect(base + 0x5F40, arrowRect);
    setControlRect(base + 0x6098, arrowRect);
}

void zoomHudMinimapImageDraw(void* image, int* x, int* y, int* width, int* height) {
    UNREFERENCED_PARAMETER(image);

    if (!x || !y || !width || !height || (!g_drawActive && !isMinimapViewportActive())) {
        return;
    }

    const int originalWidth = *width;
    const int originalHeight = *height;
    if (originalWidth < MinimapTextureSize ||
        originalHeight < MinimapAtlasHalfHeight ||
        originalWidth > 4096 ||
        originalHeight > 4096) {
        return;
    }

    const int scale = minimapScale();
    const int viewportCenter = (MinimapViewportSize * scale) / 2;
    *x = viewportCenter + ((*x - viewportCenter) * scale);
    *y = viewportCenter + ((*y - viewportCenter) * scale);
    *width = originalWidth * scale;
    *height = originalHeight * scale;
}

void beginHudMinimapGridZoom(void* hud, Rect* rect) {
    if (!hud || !rect) {
        return;
    }

    g_drawActive = true;

    const bool readWidth = safeReadFloat(
        reinterpret_cast<const void*>(AreaMapViewportWidthAddress),
        g_areaMapViewportWidthBeforeGrid);
    const bool readHeight = safeReadFloat(
        reinterpret_cast<const void*>(AreaMapViewportHeightAddress),
        g_areaMapViewportHeightBeforeGrid);
    g_haveAreaMapViewportBeforeGrid = readWidth && readHeight;

    writeMemory(reinterpret_cast<void*>(AreaMapViewportWidthAddress),
        &AreaMapViewportWidth,
        sizeof(AreaMapViewportWidth));
    writeMemory(reinterpret_cast<void*>(AreaMapViewportHeightAddress),
        &AreaMapViewportHeight,
        sizeof(AreaMapViewportHeight));

    const int scale = minimapScale();
    const int viewportSize = MinimapViewportSize * scale;
    const int gridBasis = MinimapViewportSize;
    const int centerShift = (viewportSize - gridBasis) / 2;
    char* base = static_cast<char*>(hud);

    rect->left -= centerShift;
    rect->top -= centerShift;
    writeMemory(base + 0x6088, &gridBasis, sizeof(gridBasis));
    writeMemory(base + 0x608C, &gridBasis, sizeof(gridBasis));
}

void endHudMinimapGridZoom(void* hud) {
    restoreAreaMapViewportAfterGrid();

    if (!hud) {
        g_drawActive = false;
        return;
    }

    const int viewportSize = MinimapViewportSize * minimapScale();
    char* base = static_cast<char*>(hud);
    writeMemory(base + 0x6088, &viewportSize, sizeof(viewportSize));
    writeMemory(base + 0x608C, &viewportSize, sizeof(viewportSize));
    g_drawActive = false;
}

}
