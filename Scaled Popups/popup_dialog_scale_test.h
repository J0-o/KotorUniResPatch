#pragma once

#include <windows.h>

namespace PopupDialogScaleTest {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void scaleCenteredPopup(void* owner, DWORD* returnAddressSlot);
void scaleLayoutPopup(void* owner);
void scaleLateResolutionPopup(void* owner);
void scaleStatusSummarySetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot);
void scaleMessageBoxButtonSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot);
void scaleMessageBoxAfterFix(void* owner);

}
