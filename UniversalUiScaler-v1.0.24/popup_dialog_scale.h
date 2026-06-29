#pragma once

#include <windows.h>

namespace PopupDialogScale {

struct Rect {
    int left;
    int top;
    int width;
    int height;
};

void scalePopupDialogPanel(void* owner);
void scaleStatusSummarySetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot);
void scaleMessageBoxButtonSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot);
void scaleMessageBoxLabelSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot);
void scaleMessageBoxAfterFix(void* owner);

}
