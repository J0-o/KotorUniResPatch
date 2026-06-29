#pragma once

#include <windows.h>

namespace HudMinimapScale {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void scaleHudMinimapExtent(Rect* rect, DWORD* stack);
void prepareHudMinimapScale(void* hud, int* mapX, int* mapY, int* rectWidth, int* rectHeight);
void zoomHudMinimapImageDraw(void* image, int* x, int* y, int* width, int* height);
void beginHudMinimapGridZoom(void* hud, Rect* rect);
void endHudMinimapGridZoom(void* hud);

}
