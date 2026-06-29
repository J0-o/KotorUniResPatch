#pragma once

#include <windows.h>

namespace ClassSelectionLayoutConstants {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void patchClassSelectionLayoutRects(void* ownerStackSlot, void* currentSlotMarker, void* baseRectStack, void* wrapperRectStack);
void patchInitialClassSelectionRects(void* owner);

}
