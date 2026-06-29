#pragma once

#include <windows.h>

namespace WidescreenUiScale {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;

void prepareMenuMapScale(void* map);
void prepareMenuMapIconMaterials();
void prepareMenuMapDraw(void* map, int* width);
void prepareMenuMapMarkerDraw(Rect* rect);
void prepareAreaMapDimensionsForScreen();

}
