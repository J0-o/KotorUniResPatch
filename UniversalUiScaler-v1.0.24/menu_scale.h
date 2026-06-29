#pragma once

#include <windows.h>

namespace MenuScale {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void scaleMenuPanelTree(void* panel);
void scalePazaakGameCards(void* pazaakGame);

}
