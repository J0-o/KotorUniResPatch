#pragma once

#include <windows.h>
#include <stdio.h>
#include <string.h>

namespace WidescreenUiScale {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

extern volatile int* const ScreenWidth;
extern volatile int* const ScreenHeight;

constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;
constexpr int StatusSummaryTextButtonGap = 16;

extern int g_guiBaseWidth;
extern int g_guiBaseHeight;
extern int g_guiTargetWidth;
extern int g_guiTargetHeight;
extern bool g_hudScale;
extern bool g_classSelectionScale;
extern bool g_areaMapOverlayScale;
extern Rect g_areaMapSourceViewport;
extern Rect g_areaMapTargetViewport;
extern int g_areaMapScale;
extern bool g_areaMapDimensionsPrepared;

bool writeMemory(void* address, const void* replacement, size_t size);
void writeInt(int address, int value);
void setControlRect(void* control, const Rect& rect);

int screenWidth();
int screenHeight();
bool isBaseResolution();
int scaleCoordinate(int value, int target, int source);
int integerScaleThatFits(int sourceWidth, int sourceHeight, int targetWidth, int targetHeight);
void patchCenteringConstants(int width, int height);
void setScale(int sourceWidth, int sourceHeight, int scaleBasisWidth, int scaleBasisHeight, bool updateCentering);
void setScreenScale(int sourceWidth, int sourceHeight, bool updateCentering);
void setHeightBasisScale(int sourceWidth, int sourceHeight, int heightBasis, bool updateCentering);
void setHudScale(int sourceWidth, int sourceHeight);
void setIdentityScale(int sourceWidth, int sourceHeight, bool updateCentering);

bool safeReadDword(const void* address, DWORD& value);
bool memoryContainsText(const void* address, int size, const char* needle);

bool gffContextContains(DWORD gffContext, const char* token);
bool callContextContains(DWORD* stack, const char* token);

bool isHudMinimapRect(const Rect& rect);
bool isHudMinimapControl(const Rect& rect, DWORD* stack);
bool applyHudMinimapIntegerScale(Rect* rect, DWORD* stack);
bool isMenuMapViewportRect(const Rect& rect);
bool isMenuMapViewportControl(const Rect& rect, DWORD* stack);
void resetAreaMapOverlayTransform();
void activateAreaMapOverlayTransformForScreen();
bool applyAreaMapOverlayTransform(Rect* rect, DWORD* stack);
void prepareAreaMapDimensionsForScreen();
void prepareAreaMapScale();
void prepareHudMinimapMapCoordinates(void* areaMap);
void prepareHudMinimapRenderCoordinates();
void adjustHudMinimapCenterCoordinates(void* hud, int* mapX, int* mapY);
void adjustHudMinimapFogGridDimensions(void* hud, int* mapWidth, int* mapHeight);
void anchorUnscaledMenuMapViewport(Rect* rect);
int hudRightOffset();
bool isHudRightAnchored(const Rect& rect);

bool isClassSelectionRootContext(DWORD* stack);
bool isHudRootContext(DWORD* stack);
bool isContainerRootContext(DWORD* stack);
bool isQuickPanelRootContext(DWORD* stack);
bool isPopupRootContext(DWORD* stack);
bool isFadeRootContext(DWORD* stack);
bool isRootLoadContext(DWORD* stack);
bool isTopTabRoot(const Rect& rect, DWORD* stack);

bool isClassSelectionPickerRect(const Rect& rect);
bool applyGuiExtentEdit(Rect* rect);

void scaleStatusSummaryParentFrame(void* control, const Rect& childRect);
void copyStatusSummaryCachedRects(void* control, const Rect& rect);

}
