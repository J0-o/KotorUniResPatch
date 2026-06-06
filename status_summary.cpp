#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Status summary popup layout fix

void scaleStatusSummaryParentFrame(void* control, const Rect& childRect) {
    if (!control) {
        return;
    }

    DWORD parentAddress = 0;
    if (!safeReadDword(reinterpret_cast<const char*>(control) + 0x34, parentAddress) || !parentAddress) {
        return;
    }

    Rect* parent = reinterpret_cast<Rect*>(parentAddress + 0x04);
    const int childBottom = childRect.top + childRect.height;
    const int neededHeight = childBottom + 18;

    if (parent->height < neededHeight) {
        parent->height = neededHeight;
    }
}

void copyStatusSummaryCachedRects(void* control, const Rect& rect) {
    if (!control) {
        return;
    }

    const int offsets[] = { 0x5C, 0x6C, 0xD0, 0xE0, 0x154 };
    char* base = static_cast<char*>(control);

    for (size_t i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
        Rect* cached = reinterpret_cast<Rect*>(base + offsets[i]);
        const bool plausible =
            cached->width > 0 &&
            cached->height > 0 &&
            cached->width < 1000 &&
            cached->height < 1000 &&
            cached->left > -1000 &&
            cached->left < 2000 &&
            cached->top > -1000 &&
            cached->top < 2000;

        if (plausible) {
            *cached = rect;
        }
    }
}

}
