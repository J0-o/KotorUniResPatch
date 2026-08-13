#include "popup_dialog_scale_test.h"

namespace PopupDialogScaleTest {

namespace {

constexpr DWORD ConfirmVtable = 0x0074FDB0;
constexpr DWORD DerivedConfirmVtable = 0x007513F8;
constexpr DWORD BarkBubbleVtable = 0x00755C60;
constexpr DWORD PauseVtable = 0x00756DC8;
constexpr DWORD DebugVtable = 0x00756B30;
constexpr DWORD DebugAltVtable = 0x00757A78;
constexpr DWORD ResolutionVtable = 0x00758348;
constexpr DWORD SkillInfoTooltipVtable = 0x00757940;
constexpr DWORD SaveNameVtable = 0x007576D0;
constexpr DWORD StatusSummaryVtable = 0x0074FF68;

constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;
constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;
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

bool safeReadInt(const void* address, int& value) {
    __try {
        value = *reinterpret_cast<const int*>(address);
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

int scaleValue(int value, int target, int source) {
    if (target <= 0 || source <= 0) {
        return value;
    }

    return static_cast<int>((static_cast<long long>(value) * target) / source);
}

int screenHeight() {
    int height = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenHeightAddress), height) || height <= 0) {
        return BaseHeight;
    }

    return height;
}

int screenWidth() {
    int width = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenWidthAddress), width) || width <= 0) {
        return BaseWidth;
    }

    return width;
}

int menuWidth() {
    return scaleValue(BaseWidth, screenHeight(), BaseHeight);
}

Rect scaledRect(const Rect& rect) {
    const int width = menuWidth();
    return {
        scaleValue(rect.left, width, BaseWidth),
        scaleValue(rect.top, screenHeight(), BaseHeight),
        scaleValue(rect.width, width, BaseWidth),
        scaleValue(rect.height, screenHeight(), BaseHeight),
    };
}

Rect scaledCenteredRect(const Rect& rect) {
    const int centerX = rect.left + (rect.width / 2);
    const int centerY = rect.top + (rect.height / 2);
    const int scaledWidth = scaleValue(rect.width, menuWidth(), BaseWidth);
    const int scaledHeight = scaleValue(rect.height, screenHeight(), BaseHeight);
    return {
        centerX - (scaledWidth / 2),
        centerY - (scaledHeight / 2),
        scaledWidth,
        scaledHeight,
    };
}

void scaleStatusButton(char* control, Rect& rect) {
    if (!control) {
        return;
    }

    char* owner = control - StatusButtonOffset;
    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || vtable != StatusSummaryVtable) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    const int centerX = rect.left + (rect.width / 2);
    const int width = scaleValue(StatusButtonBaseWidth, menuWidth(), BaseWidth);
    const int height = scaleValue(StatusButtonBaseHeight, screenHeight(), BaseHeight);
    const int bottomMargin = scaleValue(
        StatusButtonBaseBottomMargin, screenHeight(), BaseHeight);
    rect.left = centerX - (width / 2);
    rect.top = root->height - height - bottomMargin;
    rect.width = width;
    rect.height = height;
}

bool isConfirmMessageBoxVtable(DWORD vtable) {
    return vtable == ConfirmVtable || vtable == DerivedConfirmVtable;
}

bool isSkillInfoTooltipVtable(DWORD vtable) {
    return vtable == SkillInfoTooltipVtable;
}

bool isCenteredPopupCall(DWORD vtable, DWORD returnAddress) {
    if ((vtable == ConfirmVtable || vtable == DerivedConfirmVtable) &&
        returnAddress == ConfirmCenterReturn) {
        return true;
    }

    return (vtable == DebugVtable && returnAddress == DebugCenterReturn) ||
        (vtable == SaveNameVtable && returnAddress == SaveNameCenterReturn) ||
        (vtable == SkillInfoTooltipVtable && returnAddress == SkillInfoCenterReturn) ||
        (vtable == DebugAltVtable && returnAddress == DebugAltCenterReturn);
}

bool isLayoutPopupVtable(DWORD vtable) {
    return vtable == BarkBubbleVtable || vtable == PauseVtable;
}

bool isMessageBoxButtonSetExtentReturn(DWORD returnAddress) {
    return returnAddress == MessageBoxOkButtonFinalReturn ||
        returnAddress == MessageBoxCancelButtonFinalReturn;
}

char* messageBoxOwnerFromButton(char* control) {
    DWORD vtable = 0;
    char* owner = control - MessageBoxOkButtonOffset;
    if (safeReadDword(owner, vtable) && isConfirmMessageBoxVtable(vtable)) {
        return owner;
    }

    owner = control - MessageBoxCancelButtonOffset;
    if (safeReadDword(owner, vtable) && isConfirmMessageBoxVtable(vtable)) {
        return owner;
    }

    return nullptr;
}

int scaledMessageBoxButtonWidth(char* control, const Rect& rect) {
    int width = scaleValue(rect.width, menuWidth(), BaseWidth);
    char* owner = messageBoxOwnerFromButton(control);
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

void scaleMessageBoxFrameControl(char* owner) {
    char* control = owner + MessageBoxFrameOffset;
    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (!hasUsefulRect(*root) || !hasUsefulRect(*rect)) {
        return;
    }

    if (rect->width >= root->width / 2) {
        return;
    }

    callControlSetRect(control, scaledRect(*rect));
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

void scaleControl(char* control) {
    if (!control) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (hasUsefulRect(*rect)) {
        callControlSetRect(control, scaledRect(*rect));
    }
}

void scalePanelControls(char* panel) {
    DWORD panelVtable = 0;
    const bool isConfirm = safeReadDword(panel, panelVtable) &&
        isConfirmMessageBoxVtable(panelVtable);
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
            if (isConfirm && reinterpret_cast<char*>(child) == panel + MessageBoxFrameIconOffset) {
                continue;
            }

            scaleControl(reinterpret_cast<char*>(child));
        }
    }
}

void scaleSkillInfoTooltipControls(char* owner) {
    scaleControl(owner + SkillInfoListOffset);
    scaleControl(owner + SkillInfoTitleOffset);
    scaleControl(owner + SkillInfoOkButtonOffset);

    for (int index = 0; index < SkillInfoRowCount; ++index) {
        scaleControl(owner + SkillInfoRowOffset + (SkillInfoRowStride * index));
    }
}

bool isStatusSummarySetRectReturn(DWORD returnAddress) {
    return returnAddress == StatusSummaryRootReturn;
}

}

void scaleCenteredPopup(void* ownerPtr, DWORD* returnAddressSlot) {
    char* owner = static_cast<char*>(ownerPtr);
    DWORD returnAddress = 0;
    if (!owner ||
        !returnAddressSlot ||
        !safeReadDword(returnAddressSlot, returnAddress)) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) ||
        !isCenteredPopupCall(vtable, returnAddress)) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    if (isSkillInfoTooltipVtable(vtable)) {
        scaleSkillInfoTooltipControls(owner);
        callControlSetRect(owner, scaledRect(*root));
    }
    else {
        scalePanelControls(owner);
        callControlSetRect(owner, scaledRect(*root));
    }
}

void scaleLayoutPopup(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || !isLayoutPopupVtable(vtable)) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    scalePanelControls(owner);
    Rect scaledRoot = scaledRect(*root);
    if (vtable == BarkBubbleVtable) {
        scaledRoot.left = (screenWidth() - scaledRoot.width) / 2;
    }
    callControlSetRect(owner, scaledRoot);
}

void scaleLateResolutionPopup(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || vtable != ResolutionVtable) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    scalePanelControls(owner);
    callControlSetRect(owner, scaledRect(*root));
}

void scaleStatusSummarySetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    UNREFERENCED_PARAMETER(control);

    if (!returnAddressSlot || !rectPointerSlot) {
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
        *rect = scaledCenteredRect(*rect);
        return;
    }

}

void scaleMessageBoxButtonSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
    DWORD returnAddress = 0;
    DWORD rectAddress = 0;
    if (!returnAddressSlot ||
        !rectPointerSlot ||
        !safeReadDword(returnAddressSlot, returnAddress) ||
        !safeReadDword(rectPointerSlot, rectAddress) ||
        rectAddress == 0) {
        return;
    }

    Rect* rect = reinterpret_cast<Rect*>(rectAddress);
    if (returnAddress == StatusSummaryLowerReturn &&
        hasUsefulRect(*rect)) {
        scaleStatusButton(static_cast<char*>(control), *rect);
        return;
    }

    if (isMessageBoxButtonSetExtentReturn(returnAddress) &&
        hasUsefulRect(*rect)) {
        const int centerX = rect->left + (rect->width / 2);
        rect->width = scaledMessageBoxButtonWidth(static_cast<char*>(control), *rect);
        rect->left = centerX - (rect->width / 2);
    }
}

void scaleMessageBoxAfterFix(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || !isConfirmMessageBoxVtable(vtable)) {
        return;
    }

    scaleMessageBoxFrameControl(owner);
}

}
