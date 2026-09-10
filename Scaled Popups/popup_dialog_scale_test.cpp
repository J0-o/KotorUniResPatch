#include "popup_dialog_scale_test.h"
#include "../Common/ResolutionScale.h"

namespace PopupDialogScaleTest {

namespace {

constexpr DWORD ConfirmCenterReturn = 0x00626FF8;
constexpr DWORD DebugCenterReturn = 0x006BDDBB;
constexpr DWORD SaveNameCenterReturn = 0x006CAFFD;
constexpr DWORD SkillInfoCenterReturn = 0x006CE9A9;
constexpr DWORD DebugAltCenterReturn = 0x006CF6CB;
constexpr DWORD StatusButtonOffset = 0x1980;
constexpr int StatusButtonBaseWidth = 100;
constexpr int StatusButtonBaseHeight = 22;
constexpr int StatusButtonBaseBottomMargin = 10;
constexpr DWORD StatusSummaryRootReturn = 0x006262B7;
constexpr DWORD StatusSummaryLowerReturn = 0x00626301;
constexpr DWORD MessageBoxOkButtonFinalReturn = 0x0062588D;
constexpr DWORD MessageBoxCancelButtonFinalReturn = 0x006258DB;
constexpr DWORD MessageBoxFrameOffset = 0x74;
constexpr DWORD MessageBoxFrameIconOffset = 0x1B4;
constexpr DWORD MessageBoxOkButtonOffset = 0x2F4;
constexpr DWORD MessageBoxCancelButtonOffset = 0x4B8;
constexpr DWORD SkillInfoListOffset = 0x64;
constexpr DWORD SkillInfoTitleOffset = 0x344;
constexpr DWORD SkillInfoOkButtonOffset = 0x484;
constexpr DWORD SkillInfoRowOffset = 0x648;
constexpr DWORD SkillInfoRowStride = 0x310;
constexpr int SkillInfoRowCount = 10;
constexpr DWORD MaximumPanelChildren = 1024;

struct ControlRectSnapshot {
    char* control;
    Rect rect;
};

struct ResolutionPopupSnapshot {
    char* owner;
    Rect root;
    ControlRectSnapshot children[MaximumPanelChildren];
    DWORD childCount;
    unsigned int layoutGeneration;
};

ResolutionPopupSnapshot resolutionPopupSnapshot = {};

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

bool hasUsefulRect(const Rect& rect) {
    return rect.width > 0 && rect.height > 0 &&
        rect.width < 8192 && rect.height < 8192;
}

Rect scaledRect(const Rect& rect, const UniversalScaleState& scale) {
    return {
        scaleContentValue(rect.left, scale),
        scaleContentValue(rect.top, scale),
        scaleContentValue(rect.width, scale),
        scaleContentValue(rect.height, scale),
    };
}

Rect scaledCenteredRect(const Rect& rect, const UniversalScaleState& scale) {
    const int centerX = rect.left + (rect.width / 2);
    const int centerY = rect.top + (rect.height / 2);
    const int scaledWidth = scaleContentValue(rect.width, scale);
    const int scaledHeight = scaleContentValue(rect.height, scale);
    return {
        centerX - (scaledWidth / 2),
        centerY - (scaledHeight / 2),
        scaledWidth,
        scaledHeight,
    };
}

void scaleStatusButton(char* control, Rect& rect, const UniversalScaleState& scale) {
    if (!control) {
        return;
    }

    char* owner = control - StatusButtonOffset;
    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    const int centerX = rect.left + (rect.width / 2);
    const int width = scaleContentValue(StatusButtonBaseWidth, scale);
    const int height = scaleContentValue(StatusButtonBaseHeight, scale);
    const int bottomMargin = scaleContentValue(StatusButtonBaseBottomMargin, scale);
    rect.left = centerX - (width / 2);
    rect.top = root->height - height - bottomMargin;
    rect.width = width;
    rect.height = height;
}

bool isCenteredPopupCall(DWORD returnAddress) {
    return returnAddress == ConfirmCenterReturn ||
        returnAddress == DebugCenterReturn ||
        returnAddress == SaveNameCenterReturn ||
        returnAddress == SkillInfoCenterReturn ||
        returnAddress == DebugAltCenterReturn;
}

bool isMessageBoxButtonSetExtentReturn(DWORD returnAddress) {
    return returnAddress == MessageBoxOkButtonFinalReturn ||
        returnAddress == MessageBoxCancelButtonFinalReturn;
}

char* messageBoxOwnerFromButton(char* control, DWORD returnAddress) {
    if (returnAddress == MessageBoxOkButtonFinalReturn) {
        return control - MessageBoxOkButtonOffset;
    }

    if (returnAddress == MessageBoxCancelButtonFinalReturn) {
        return control - MessageBoxCancelButtonOffset;
    }

    return nullptr;
}

int scaledMessageBoxButtonWidth(char* control, const Rect& rect, DWORD returnAddress,
                                const UniversalScaleState& scale) {
    int width = scaleContentValue(rect.width, scale);
    char* owner = messageBoxOwnerFromButton(control, returnAddress);
    if (!owner) {
        return width;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return width;
    }

    const int margin = rect.left > 0 ? rect.left : 0;
    const int maxWidth = root->width - (margin * 2);
    if (maxWidth > 0 && width > maxWidth) {
        width = maxWidth;
    }

    return width;
}

void callControlSetRect(char* control, const Rect& rect);

void scaleMessageBoxFrameControl(char* owner, const UniversalScaleState& scale) {
    char* control = owner + MessageBoxFrameOffset;
    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (!hasUsefulRect(*root) || !hasUsefulRect(*rect)) {
        return;
    }

    if (rect->width >= root->width / 2) {
        return;
    }

    callControlSetRect(control, scaledRect(*rect, scale));
}

void callControlSetRect(char* control, const Rect& rect) {
    if (!control) {
        return;
    }

    __try {
        DWORD vtable = *reinterpret_cast<DWORD*>(control);
        DWORD setRect = *reinterpret_cast<DWORD*>(vtable + 4);
        if (setRect != 0) {
            typedef void(__thiscall *SetRectFn)(void*, const Rect*);
            reinterpret_cast<SetRectFn>(setRect)(control, &rect);
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    __try {
        *reinterpret_cast<Rect*>(control + sizeof(DWORD)) = rect;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

void scaleControl(char* control, const UniversalScaleState& scale) {
    if (!control) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (hasUsefulRect(*rect)) {
        callControlSetRect(control, scaledRect(*rect, scale));
    }
}

void scalePanelControls(char* panel, const UniversalScaleState& scale,
                        bool skipMessageBoxIcon = false) {
    DWORD childrenData = 0;
    DWORD childrenSize = 0;
    if (!safeReadDword(panel + 0x20, childrenData) ||
        !safeReadDword(panel + 0x24, childrenSize) ||
        childrenData == 0 ||
        childrenSize > 1024) {
        return;
    }

    for (DWORD i = 0; i < childrenSize; ++i) {
        DWORD child = 0;
        if (safeReadDword(reinterpret_cast<const void*>(childrenData + (i * sizeof(DWORD))), child) &&
            child != 0) {
            if (skipMessageBoxIcon &&
                reinterpret_cast<char*>(child) == panel + MessageBoxFrameIconOffset) {
                continue;
            }

            scaleControl(reinterpret_cast<char*>(child), scale);
        }
    }
}

bool captureResolutionPopup(char* owner,
                            const UniversalScaleState& scale) {
    DWORD childrenData = 0;
    DWORD childrenSize = 0;
    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root) ||
        !safeReadDword(owner + 0x20, childrenData) ||
        !safeReadDword(owner + 0x24, childrenSize) ||
        childrenData == 0 ||
        childrenSize > MaximumPanelChildren) {
        return false;
    }

    resolutionPopupSnapshot.owner = owner;
    resolutionPopupSnapshot.root = *root;
    resolutionPopupSnapshot.childCount = 0;
    resolutionPopupSnapshot.layoutGeneration = scale.layoutGeneration;

    for (DWORD i = 0; i < childrenSize; ++i) {
        DWORD childAddress = 0;
        if (!safeReadDword(
                reinterpret_cast<const void*>(childrenData + (i * sizeof(DWORD))),
                childAddress) ||
            childAddress == 0) {
            continue;
        }

        char* child = reinterpret_cast<char*>(childAddress);
        Rect* rect = reinterpret_cast<Rect*>(child + sizeof(DWORD));
        if (!hasUsefulRect(*rect)) {
            continue;
        }

        ControlRectSnapshot& snapshot = resolutionPopupSnapshot.children[
            resolutionPopupSnapshot.childCount++];
        snapshot.control = child;
        snapshot.rect = *rect;
    }

    return true;
}

void applyResolutionPopupSnapshot(const UniversalScaleState& scale) {
    ResolutionPopupSnapshot& snapshot = resolutionPopupSnapshot;
    if (!snapshot.owner) {
        return;
    }

    for (DWORD i = 0; i < snapshot.childCount; ++i) {
        const ControlRectSnapshot& child = snapshot.children[i];
        callControlSetRect(child.control, scaledRect(child.rect, scale));
    }

    callControlSetRect(snapshot.owner, scaledRect(snapshot.root, scale));
    snapshot.layoutGeneration = scale.layoutGeneration;
}

void scaleSkillInfoTooltipControls(char* owner, const UniversalScaleState& scale) {
    scaleControl(owner + SkillInfoListOffset, scale);
    scaleControl(owner + SkillInfoTitleOffset, scale);
    scaleControl(owner + SkillInfoOkButtonOffset, scale);

    for (int index = 0; index < SkillInfoRowCount; ++index) {
        scaleControl(owner + SkillInfoRowOffset + (SkillInfoRowStride * index), scale);
    }
}

bool isStatusSummarySetRectReturn(DWORD returnAddress) {
    return returnAddress == StatusSummaryRootReturn;
}

}

void scaleCenteredPopup(void* ownerPtr, DWORD* returnAddressSlot) {
    const UniversalScaleState* scale = ResolutionScale::get();
    char* owner = static_cast<char*>(ownerPtr);
    DWORD returnAddress = 0;
    if (!owner ||
        !returnAddressSlot ||
        !scale ||
        !safeReadDword(returnAddressSlot, returnAddress) ||
        !isCenteredPopupCall(returnAddress)) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    if (returnAddress == SkillInfoCenterReturn) {
        scaleSkillInfoTooltipControls(owner, *scale);
        callControlSetRect(owner, scaledRect(*root, *scale));
    }
    else {
        scalePanelControls(owner, *scale, returnAddress == ConfirmCenterReturn);
        callControlSetRect(owner, scaledRect(*root, *scale));
    }
}

void scaleLayoutPopup(void* ownerPtr, bool centerHorizontally) {
    const UniversalScaleState* scale = ResolutionScale::get();
    char* owner = static_cast<char*>(ownerPtr);
    if (!scale || !owner) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    scalePanelControls(owner, *scale);
    Rect scaledRoot = scaledRect(*root, *scale);
    if (centerHorizontally) {
        scaledRoot.left = (scale->screenWidth - scaledRoot.width) / 2;
    }
    callControlSetRect(owner, scaledRoot);
}

void scaleLateResolutionPopup(void* ownerPtr) {
    const UniversalScaleState* scale = ResolutionScale::get();
    char* owner = static_cast<char*>(ownerPtr);
    if (!scale || !owner) {
        return;
    }

    if (!captureResolutionPopup(owner, *scale)) {
        return;
    }

    applyResolutionPopupSnapshot(*scale);
}

void refreshLateResolutionPopup(void* ownerPtr) {
    const UniversalScaleState* scale = ResolutionScale::get();
    char* owner = static_cast<char*>(ownerPtr);
    if (!scale ||
        !owner ||
        resolutionPopupSnapshot.owner != owner ||
        resolutionPopupSnapshot.layoutGeneration == scale->layoutGeneration) {
        return;
    }

    applyResolutionPopupSnapshot(*scale);
}

void refreshTrackedPopups() {
    refreshLateResolutionPopup(resolutionPopupSnapshot.owner);
}

void scaleStatusSummarySetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    UNREFERENCED_PARAMETER(control);

    const UniversalScaleState* scale = ResolutionScale::get();
    if (!scale || !returnAddressSlot || !rectPointerSlot) {
        return;
    }

    DWORD returnAddress = 0;
    DWORD rectAddress = 0;
    if (!safeReadDword(returnAddressSlot, returnAddress) ||
        !safeReadDword(rectPointerSlot, rectAddress) ||
        !isStatusSummarySetRectReturn(returnAddress) ||
        rectAddress == 0) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(rectAddress);
    if (!hasUsefulRect(*rect)) {
        return;
    }

    // FUN_00625C60 derives the root from measured text. Scale those calculated
    // dimensions responsively while preserving the center chosen by the game.
    if (returnAddress == StatusSummaryRootReturn) {
        *rect = scaledCenteredRect(*rect, *scale);
        return;
    }

}

void scaleMessageBoxButtonSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    const UniversalScaleState* scale = ResolutionScale::get();
    DWORD returnAddress = 0;
    DWORD rectAddress = 0;
    if (!scale ||
        !returnAddressSlot ||
        !rectPointerSlot ||
        !safeReadDword(returnAddressSlot, returnAddress) ||
        !safeReadDword(rectPointerSlot, rectAddress) ||
        rectAddress == 0) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(rectAddress);
    if (returnAddress == StatusSummaryLowerReturn &&
        hasUsefulRect(*rect)) {
        scaleStatusButton(static_cast<char*>(control), *rect, *scale);
        return;
    }

    if (isMessageBoxButtonSetExtentReturn(returnAddress) &&
        hasUsefulRect(*rect)) {
        const int centerX = rect->left + (rect->width / 2);
        rect->width = scaledMessageBoxButtonWidth(
            static_cast<char*>(control), *rect, returnAddress, *scale);
        rect->left = centerX - (rect->width / 2);
    }
}

void scaleMessageBoxAfterFix(void* ownerPtr) {
    const UniversalScaleState* scale = ResolutionScale::get();
    char* owner = static_cast<char*>(ownerPtr);
    if (!scale || !owner) {
        return;
    }

    scaleMessageBoxFrameControl(owner, *scale);
}

}
