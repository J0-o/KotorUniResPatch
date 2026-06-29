#include "class_selection_layout_constants.h"

namespace ClassSelectionLayoutConstants {

namespace {

constexpr DWORD ClassSelectionOwnerVtable = 0x00758020;
constexpr DWORD ClassSlotWrapperVtable = 0x00752E30;
constexpr DWORD SlotArrayOffset = 0x6C;
constexpr DWORD SlotMarkerOffset = 0x2C0;
constexpr DWORD SlotStride = 0x25C;
constexpr int SlotCount = 6;
constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;
constexpr int BaseBoxWidth = 94;
constexpr int BaseBoxHeight = 240;
constexpr int BaseBoxLefts[] = {
    77,
    190,
    299,
    406,
    516,
    625,
};
constexpr DWORD SlotRectOffsets[] = {
    0x000,
    0x06C,
    0x0E0,
    0x154,
    0x1C4,
    0x220,
};

bool safeReadDword(const void* address, DWORD& value) {
    __try {
        value = *reinterpret_cast<const DWORD*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0;
        return false;
    }
}

bool safeReadRect(const void* address, Rect& rect) {
    __try {
        rect = *reinterpret_cast<const Rect*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        rect = {};
        return false;
    }
}

bool writeRect(void* address, const Rect& rect) {
    __try {
        *reinterpret_cast<Rect*>(address) = rect;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool isUsefulRootRect(const Rect& rect) {
    return rect.left == 0 &&
        rect.top == 0 &&
        rect.width >= 640 &&
        rect.height >= 480 &&
        rect.width <= 8192 &&
        rect.height <= 8192;
}

int scaleValue(int value, int target, int source) {
    if (target <= 0 || source <= 0) {
        return value;
    }

    int scaled = static_cast<int>((static_cast<long long>(value) * target) / source);
    return scaled > 1 ? scaled : 1;
}

Rect scaledSlotRect(int slotIndex, const Rect& root) {
    const int scaledCanvasWidth = scaleValue(BaseWidth, root.height, BaseHeight);
    const int offsetX = (root.width - scaledCanvasWidth) / 2;
    const int width = scaleValue(BaseBoxWidth, root.height, BaseHeight);
    const int height = scaleValue(BaseBoxHeight, root.height, BaseHeight);

    return {
        offsetX + scaleValue(BaseBoxLefts[slotIndex], root.height, BaseHeight),
        (root.height - height) / 2,
        width,
        height,
    };
}

}

void patchClassSelectionLayoutRects(void* ownerStackSlot, void* currentSlotMarker, void* baseRectStack, void* wrapperRectStack) {
    DWORD ownerValue = 0;
    if (!safeReadDword(ownerStackSlot, ownerValue) || ownerValue == 0) {
        return;
    }

    char* owner = reinterpret_cast<char*>(ownerValue);
    DWORD ownerVtable = 0;
    if (!safeReadDword(owner, ownerVtable) || ownerVtable != ClassSelectionOwnerVtable) {
        return;
    }

    char* marker = static_cast<char*>(currentSlotMarker);
    const ptrdiff_t markerDelta = marker - (owner + SlotMarkerOffset);
    if (markerDelta < 0 || (markerDelta % SlotStride) != 0) {
        return;
    }

    const int slotIndex = static_cast<int>(markerDelta / SlotStride);
    if (slotIndex < 0 || slotIndex >= SlotCount) {
        return;
    }

    Rect root = {};
    if (!safeReadRect(owner + sizeof(DWORD), root) || !isUsefulRootRect(root)) {
        return;
    }

    const Rect target = scaledSlotRect(slotIndex, root);
    writeRect(baseRectStack, target);
    writeRect(wrapperRectStack, target);
}

void patchInitialClassSelectionRects(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (owner == nullptr) {
        return;
    }

    DWORD ownerVtable = 0;
    if (!safeReadDword(owner, ownerVtable) || ownerVtable != ClassSelectionOwnerVtable) {
        return;
    }

    Rect root = {};
    if (!safeReadRect(owner + sizeof(DWORD), root) || !isUsefulRootRect(root)) {
        return;
    }

    for (int slotIndex = 0; slotIndex < SlotCount; ++slotIndex) {
        char* slot = owner + SlotArrayOffset + (SlotStride * slotIndex);
        char* wrapper = slot + 0x1C4;

        DWORD wrapperVtable = 0;
        if (!safeReadDword(wrapper, wrapperVtable) || wrapperVtable != ClassSlotWrapperVtable) {
            continue;
        }

        const Rect target = scaledSlotRect(slotIndex, root);
        for (int rectIndex = 0; rectIndex < static_cast<int>(sizeof(SlotRectOffsets) / sizeof(SlotRectOffsets[0])); ++rectIndex) {
            writeRect(slot + SlotRectOffsets[rectIndex] + sizeof(DWORD), target);
        }
    }
}

}
