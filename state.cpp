#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

volatile int* const ScreenWidth = reinterpret_cast<volatile int*>(0x0078D1D4);
volatile int* const ScreenHeight = reinterpret_cast<volatile int*>(0x0078D1D8);

int g_guiBaseWidth = BaseWidth;
int g_guiBaseHeight = BaseHeight;
int g_guiTargetWidth = BaseWidth;
int g_guiTargetHeight = BaseHeight;
bool g_hudScale = false;
bool g_classSelectionScale = false;
bool g_areaMapOverlayScale = false;
Rect g_areaMapSourceViewport = {};
Rect g_areaMapTargetViewport = {};
int g_areaMapScale = 1;
bool g_areaMapDimensionsPrepared = false;

}
