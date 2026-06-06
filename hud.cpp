#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// HUD minimap scaling fix

bool isHudMinimapRect(const Rect& rect) {
    return (rect.left == -2 && rect.top == -3 && rect.width == 136 && rect.height == 137) ||
        (rect.left == 6 && rect.top == 6 && rect.width == 512 && rect.height == 512) ||
        (rect.left == 6 && rect.top == 6 && rect.width == 120 && rect.height == 120) ||
        (rect.left == 8 && rect.top == 8 && rect.width == 118 && rect.height == 118) ||
        (rect.left == 47 && rect.top == 49 && rect.width == 32 && rect.height == 32);
}

bool isHudMinimapControl(const Rect& rect, DWORD* stack) {
    if (isHudMinimapRect(rect)) {
        return true;
    }

    return callContextContains(stack, "LBL_MAP") ||
        callContextContains(stack, "LBL_MAPBORDER") ||
        callContextContains(stack, "LBL_MAPVIEW") ||
        callContextContains(stack, "BTN_MINIMAP") ||
        callContextContains(stack, "LBL_ARROW|") ||
        callContextContains(stack, "lbl_map|") ||
        callContextContains(stack, "lbl_mapborder") ||
        callContextContains(stack, "lbl_mapview") ||
        callContextContains(stack, "btn_minimap") ||
        callContextContains(stack, "lbl_minimap") ||
        callContextContains(stack, "lbl_arrow|");
}

bool applyHudMinimapIntegerScale(Rect* rect, DWORD* stack) {
    if (!rect || !isHudMinimapControl(*rect, stack)) {
        return false;
    }

    if (isHudRightAnchored(*rect)) {
        rect->left += hudRightOffset();
    }

    return true;
}

bool isMenuMapViewportRect(const Rect& rect) {
    return (rect.left == 119 && rect.top == 148 && rect.width == 550 && rect.height == 320) ||
        (rect.left == 95 && rect.top == 118 && rect.width == 440 && rect.height == 256) ||
        (rect.width == 512 && rect.height == 256);
}

bool isMenuMapViewportControl(const Rect& rect, DWORD* stack) {
    if (isMenuMapViewportRect(rect)) {
        return true;
    }

    return callContextContains(stack, "LBL_Map|");
}

constexpr int AreaMapBaseTextureWidth = 512;
constexpr int AreaMapBaseTextureHeight = 256;
constexpr int AreaMapBaseViewportWidth = 440;
constexpr int AreaMapBaseViewportHeight = 256;
constexpr int AreaMapPanelLeft = 119;
constexpr int AreaMapPanelTop = 148;
constexpr int AreaMapPanelWidth = 550;
constexpr int AreaMapPanelHeight = 320;
constexpr int AreaMapSourceLeft = 95;
constexpr int AreaMapSourceTop = 118;
constexpr int AreaMapSourceWidth = 440;
constexpr int AreaMapSourceHeight = 256;
constexpr int HudMinimapCoordinateWidth = 120;
constexpr int HudMinimapCoordinateHeight = 240;

void writeFloat(int address, float value) {
    writeMemory(reinterpret_cast<void*>(address), &value, sizeof(value));
}

void setSharedMapCoordinateBasis(int width, int height) {
    writeFloat(0x00747748, static_cast<float>(width));
    writeFloat(0x007455D4, static_cast<float>(height));
}

void getAreaMapViewportForTarget(int targetWidth, int targetHeight, Rect* viewport, int* scale);
void getAreaMapViewportForCurrentMenu(Rect* viewport, int* scale);
void getAreaMapViewportForScreen(Rect* viewport, int* scale);

BYTE* replacementCodeForHook(int address) {
    BYTE* hookAddress = reinterpret_cast<BYTE*>(address);
    if (*hookAddress != 0xE9) {
        return nullptr;
    }

    DWORD relativeJump = 0;
    if (!safeReadDword(hookAddress + 1, relativeJump)) {
        return nullptr;
    }

    return hookAddress + 5 + static_cast<int>(relativeJump);
}

int areaMapIntegerScaleForPanel(int panelWidth, int panelHeight) {
    return integerScaleThatFits(
        AreaMapBaseViewportWidth,
        AreaMapBaseViewportHeight,
        panelWidth,
        panelHeight);
}

int areaMapMenuOffsetX() {
    const int offset = screenWidth() - g_guiTargetWidth;
    return offset > 0 ? offset / 2 : 0;
}

int areaMapMenuOffsetY() {
    const int offset = screenHeight() - g_guiTargetHeight;
    return offset > 0 ? offset / 2 : 0;
}

void setAreaMapOverlayTransform(const Rect& viewport, int scale) {
    g_areaMapOverlayScale = true;
    g_areaMapSourceViewport = { AreaMapSourceLeft, AreaMapSourceTop, AreaMapSourceWidth, AreaMapSourceHeight };
    g_areaMapTargetViewport = viewport;
    g_areaMapScale = scale;
}

void patchAreaMapReplaceCaveRaw(int drawLeft, int drawTop, int viewportWidth, int viewportHeight) {
    BYTE* hookAddress = reinterpret_cast<BYTE*>(0x006928D3);
    if (*hookAddress != 0xE9) {
        return;
    }

    DWORD relativeJump = 0;
    if (!safeReadDword(hookAddress + 1, relativeJump)) {
        return;
    }

    BYTE* code = hookAddress + 5 + static_cast<int>(relativeJump);
    writeInt(reinterpret_cast<int>(code + 1), drawLeft);
    writeInt(reinterpret_cast<int>(code + 6), drawTop);
    writeInt(reinterpret_cast<int>(code + 11), viewportHeight);
    writeInt(reinterpret_cast<int>(code + 16), viewportWidth);
}

void patchAreaMapReplaceCave(int viewportLeft, int viewportTop, int viewportWidth, int viewportHeight) {
    const int drawLeft = viewportLeft + areaMapMenuOffsetX();
    const int drawTop = viewportTop + areaMapMenuOffsetY();
    patchAreaMapReplaceCaveRaw(drawLeft, drawTop, viewportWidth, viewportHeight);
}

void patchAreaMapHitTestOrigin(int viewportLeft, int viewportTop) {
    BYTE* xOriginCode = replacementCodeForHook(0x0069330A);
    if (xOriginCode) {
        writeInt(reinterpret_cast<int>(xOriginCode + 1), viewportLeft);
    }

    BYTE* yOriginCode = replacementCodeForHook(0x0069331C);
    if (yOriginCode) {
        writeInt(reinterpret_cast<int>(yOriginCode + 1), viewportTop);
    }
}

void patchAreaMapIconMaterialSizes() {
    const int arrowSize = 0x20;
    const int circleSize = 0x10;
    const int targetSize = 0x14;

    writeInt(0x0069405B, arrowSize);
    writeInt(0x006940DC, circleSize);
    writeInt(0x0069418F, targetSize);
}

void patchAreaMapCoordinateBounds(int viewportWidth, int viewportHeight) {
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

    setSharedMapCoordinateBasis(viewportWidth, viewportHeight);
}

void patchActiveAreaMapCoordinateBounds() {
    if (!g_areaMapOverlayScale || g_areaMapScale <= 0) {
        return;
    }

    patchAreaMapCoordinateBounds(
        AreaMapBaseViewportWidth * g_areaMapScale,
        AreaMapBaseViewportHeight * g_areaMapScale);
}

void resetAreaMapOverlayTransform() {
    g_areaMapOverlayScale = false;
    g_areaMapSourceViewport = {};
    g_areaMapTargetViewport = {};
    g_areaMapScale = 1;
}

void activateAreaMapOverlayTransformForScreen() {
    Rect viewport = {};
    int scale = 1;
    getAreaMapViewportForScreen(&viewport, &scale);
    setAreaMapOverlayTransform(viewport, scale);
    patchAreaMapCoordinateBounds(
        AreaMapBaseViewportWidth * scale,
        AreaMapBaseViewportHeight * scale);
}

bool isNamedAreaMapMarker(DWORD* stack) {
    return callContextContains(stack, "whitetarget") ||
        callContextContains(stack, "lbl_itari") ||
        callContextContains(stack, "lbl_idant") ||
        callContextContains(stack, "lbl_itato") ||
        callContextContains(stack, "lbl_ikash") ||
        callContextContains(stack, "lbl_imana") ||
        callContextContains(stack, "lbl_ikorr") ||
        callContextContains(stack, "lbl_iunkn") ||
        callContextContains(stack, "lbl_strforge") ||
        callContextContains(stack, "lbl_live");
}

bool isAreaMapOverlayControl(const Rect& rect, DWORD* stack) {
    if (!g_areaMapOverlayScale || g_areaMapScale <= 0) {
        return false;
    }

    if (rect.width <= 0 || rect.height <= 0 || rect.width > 64 || rect.height > 64) {
        return false;
    }

    if (isNamedAreaMapMarker(stack)) {
        return true;
    }

    const bool insideSourceViewport =
        rect.left >= AreaMapSourceLeft &&
        rect.top >= AreaMapSourceTop &&
        rect.left + rect.width <= AreaMapSourceLeft + AreaMapSourceWidth &&
        rect.top + rect.height <= AreaMapSourceTop + AreaMapSourceHeight;
    if (insideSourceViewport) {
        return true;
    }

    return false;
}

bool applyAreaMapOverlayTransform(Rect* rect, DWORD* stack) {
    if (!rect) {
        return false;
    }

    if (!g_areaMapOverlayScale && isNamedAreaMapMarker(stack)) {
        activateAreaMapOverlayTransformForScreen();
    }

    if (!isAreaMapOverlayControl(*rect, stack)) {
        return false;
    }

    patchActiveAreaMapCoordinateBounds();

    const Rect sourceViewport = g_areaMapSourceViewport;
    const Rect targetViewport = g_areaMapTargetViewport;
    const int scale = g_areaMapScale;

    const Rect before = *rect;
    rect->left = targetViewport.left + ((before.left - sourceViewport.left) * scale);
    rect->top = targetViewport.top + ((before.top - sourceViewport.top) * scale);
    rect->width = before.width * scale;
    rect->height = before.height * scale;

    return true;
}

void patchAreaMapEngineDimensions(int scale) {
    const int textureWidth = AreaMapBaseTextureWidth * scale;
    const int textureHeight = AreaMapBaseTextureHeight * scale;
    const int viewportWidth = AreaMapBaseViewportWidth * scale;
    const int viewportHeight = AreaMapBaseViewportHeight * scale;

    writeInt(0x0069505C, textureWidth);
    writeInt(0x00695064, textureHeight);
    writeInt(0x00695082, viewportWidth);
    writeInt(0x0069508A, viewportHeight);
    patchAreaMapCoordinateBounds(viewportWidth, viewportHeight);
}

void patchAreaMapEngineScale(int viewportLeft, int viewportTop, int scale) {
    const int viewportWidth = AreaMapBaseViewportWidth * scale;
    const int viewportHeight = AreaMapBaseViewportHeight * scale;

    patchAreaMapIconMaterialSizes();
    patchAreaMapEngineDimensions(scale);
    patchAreaMapHitTestOrigin(viewportLeft, viewportTop);
    patchAreaMapReplaceCave(viewportLeft, viewportTop, viewportWidth, viewportHeight);
}

void prepareHudMinimapMapCoordinates(void* areaMap) {
    if (!areaMap) {
        return;
    }

    char* base = static_cast<char*>(areaMap);
    DWORD x = 0;
    DWORD y = 0;
    DWORD width = 0;
    DWORD height = 0;
    safeReadDword(base + 0x6080, x);
    safeReadDword(base + 0x6084, y);
    safeReadDword(base + 0x6088, width);
    safeReadDword(base + 0x608C, height);

    const bool hudMinimap =
        width > 0 &&
        height > 0 &&
        width <= 320 &&
        height <= 320 &&
        x <= 64 &&
        y <= 64;
    if (!hudMinimap) {
        return;
    }

    setSharedMapCoordinateBasis(HudMinimapCoordinateWidth, HudMinimapCoordinateHeight);
}

void prepareHudMinimapRenderCoordinates() {
    setSharedMapCoordinateBasis(HudMinimapCoordinateWidth, HudMinimapCoordinateHeight);
}

void adjustHudMinimapCenterCoordinates(void* hud, int* mapX, int* mapY) {
    if (!hud || !mapX || !mapY) {
        return;
    }

    int scale = 1;
    getAreaMapViewportForScreen(nullptr, &scale);
    if (scale <= 1) {
        return;
    }

    *mapX /= scale;
    *mapY /= scale;
}

void adjustHudMinimapFogGridDimensions(void* hud, int* mapWidth, int* mapHeight) {
    if (!hud || !mapWidth || !mapHeight) {
        return;
    }

    int scale = 1;
    getAreaMapViewportForScreen(nullptr, &scale);
    if (scale <= 1) {
        return;
    }

    const int fogGridCoordinateWidth = AreaMapBaseViewportWidth;
    const int fogGridCoordinateHeight = AreaMapBaseViewportHeight;
    setSharedMapCoordinateBasis(fogGridCoordinateWidth, fogGridCoordinateHeight);
}

void getAreaMapViewportForTarget(int targetWidth, int targetHeight, Rect* viewport, int* scale) {
    const int panelWidth = scaleCoordinate(AreaMapPanelWidth, targetWidth, BaseWidth);
    const int panelHeight = scaleCoordinate(AreaMapPanelHeight, targetHeight, BaseHeight);
    const int selectedScale = areaMapIntegerScaleForPanel(panelWidth, panelHeight);
    const int viewportWidth = AreaMapBaseViewportWidth * selectedScale;
    const int viewportHeight = AreaMapBaseViewportHeight * selectedScale;

    if (viewport) {
        viewport->left = (targetWidth - viewportWidth) / 2;
        viewport->top = (targetHeight - viewportHeight) / 2;
        viewport->width = viewportWidth;
        viewport->height = viewportHeight;
    }

    if (scale) {
        *scale = selectedScale;
    }
}

void getAreaMapViewportForCurrentMenu(Rect* viewport, int* scale) {
    getAreaMapViewportForTarget(g_guiTargetWidth, g_guiTargetHeight, viewport, scale);
}

void getAreaMapViewportForScreen(Rect* viewport, int* scale) {
    int targetWidth = screenWidth();
    int targetHeight = scaleCoordinate(480, targetWidth, 640);
    if (targetHeight > screenHeight()) {
        targetHeight = screenHeight();
        targetWidth = scaleCoordinate(640, targetHeight, 480);
    }

    getAreaMapViewportForTarget(targetWidth, targetHeight, viewport, scale);
}

void prepareAreaMapScale() {
    int scale = 1;
    Rect viewport = {};
    getAreaMapViewportForScreen(&viewport, &scale);
    setAreaMapOverlayTransform(viewport, scale);
    patchAreaMapEngineScale(viewport.left, viewport.top, scale);
    g_areaMapDimensionsPrepared = true;
}

void prepareAreaMapDimensionsForScreen() {
    int scale = 1;
    getAreaMapViewportForScreen(nullptr, &scale);
    patchAreaMapEngineDimensions(scale);
    g_areaMapDimensionsPrepared = true;
}

void anchorUnscaledMenuMapViewport(Rect* rect) {
    if (!rect) {
        return;
    }

    int scale = 1;
    Rect viewport = {};
    getAreaMapViewportForCurrentMenu(&viewport, &scale);
    setAreaMapOverlayTransform(viewport, scale);
    rect->left = viewport.left;
    rect->top = viewport.top;
    rect->width = viewport.width;
    rect->height = viewport.height;
    patchAreaMapEngineScale(rect->left, rect->top, scale);
}

int hudRightOffset() {
    const int offset = screenWidth() - g_guiTargetWidth;
    return offset > 0 ? offset : 0;
}

bool isHudRightAnchored(const Rect& rect) {
    return rect.left >= 320;
}


}
