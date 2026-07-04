#pragma once

#include <windows.h>

namespace ScaledPanels {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void scaleQuickOrCustomPanel(void* owner);
void scaleQuickPanel(void* owner);
void scaleLevelUpPanel(void* owner);
void scaleCustomPanel(void* owner);

}
