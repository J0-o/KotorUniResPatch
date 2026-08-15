#pragma once

#include <windows.h>

namespace ContainerPopupScaleTest {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void scaleContainerPanel(void* owner);

}
