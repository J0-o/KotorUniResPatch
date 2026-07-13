#include "popup_dialog_scale_test.h"

namespace PopupDialogScaleTest {

namespace {

constexpr DWORD ConfirmVtable = 0x0074FDB0;
constexpr DWORD DerivedConfirmVtable = 0x007513F8;
constexpr DWORD BarkBubbleVtable = 0x00755C60;
constexpr DWORD PauseVtable = 0x00756DC8;
constexpr DWORD AreaTransitionVtable = 0x00757508;
constexpr DWORD DebugVtable = 0x00756B30;
constexpr DWORD DebugAltVtable = 0x00757A78;
constexpr DWORD ResolutionVtable = 0x00758348;
constexpr DWORD SkillInfoTooltipVtable = 0x00757940;
constexpr DWORD StatusSummaryVtable = 0x0074FF68;

constexpr int BaseWidth = 800;
constexpr int TestScale = 2;
constexpr DWORD StatusButtonOffset = 0x1980;
constexpr int StatusButtonWidth = 200;
constexpr int StatusButtonHeight = 44;
constexpr int StatusButtonBottomMargin = 20;
constexpr DWORD StatusSummaryRootReturn = 0x006262B7;
constexpr DWORD StatusSummaryLowerReturn = 0x00626301;
constexpr DWORD MessageBoxOkButtonFinalReturn = 0x0062588D;
constexpr DWORD MessageBoxCancelButtonFinalReturn = 0x006258DB;
constexpr DWORD MessageBoxFrameIconFinalReturn = 0x00625835;
constexpr DWORD StatusSummaryIconReturn = 0x0062615C;
constexpr int StatusSummaryFirstRowTop = 10;
constexpr int StatusSummaryNativeRowSpacing = 0x25;
constexpr int StatusSummaryExtraIconSpacing = 18;
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

int menuWidth() {
    return BaseWidth * TestScale;
}

Rect scaledRect(const Rect& rect) {
    const int width = menuWidth();
    return {
        scaleValue(rect.left, width, BaseWidth),
        rect.top * TestScale,
        scaleValue(rect.width, width, BaseWidth),
        rect.height * TestScale,
    };
}

Rect scaledHeightRect(const Rect& rect) {
    return {
        rect.left * TestScale,
        rect.top * TestScale,
        rect.width * TestScale,
        rect.height * TestScale,
    };
}

Rect scaledCenteredHeightRect(const Rect& rect) {
    const int centerX = rect.left + (rect.width / 2);
    const int centerY = rect.top + (rect.height / 2);
    const int scaledWidth = rect.width * TestScale;
    const int scaledHeight = rect.height * TestScale;

    return {
        centerX - (scaledWidth / 2),
        centerY - (scaledHeight / 2),
        scaledWidth,
        scaledHeight,
    };
}

Rect doubledHeightCenteredRect(const Rect& rect) {
    const int centerY = rect.top + (rect.height / 2);
    const int scaledHeight = rect.height * TestScale;
    return {
        rect.left,
        centerY - (scaledHeight / 2),
        rect.width,
        scaledHeight,
    };
}

Rect scaledCenteredWidthRect(const Rect& rect) {
    const int centerX = rect.left + (rect.width / 2);
    const int scaledWidth = rect.width * TestScale;
    return {
        centerX - (scaledWidth / 2),
        rect.top * TestScale,
        scaledWidth,
        rect.height * TestScale,
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
    rect.left = centerX - (StatusButtonWidth / 2);
    rect.top = root->height - StatusButtonHeight - StatusButtonBottomMargin;
    rect.width = StatusButtonWidth;
    rect.height = StatusButtonHeight;
}

bool isExpectedPopupVtable(DWORD vtable) {
    return vtable == ConfirmVtable ||
        vtable == DerivedConfirmVtable ||
        vtable == BarkBubbleVtable ||
        vtable == PauseVtable ||
        vtable == AreaTransitionVtable ||
        vtable == DebugVtable ||
        vtable == DebugAltVtable ||
        vtable == ResolutionVtable ||
        vtable == SkillInfoTooltipVtable;
}

bool isConfirmMessageBoxVtable(DWORD vtable) {
    return vtable == ConfirmVtable || vtable == DerivedConfirmVtable;
}

bool isSkillInfoTooltipVtable(DWORD vtable) {
    return vtable == SkillInfoTooltipVtable;
}

bool isMessageBoxButtonSetExtentReturn(DWORD returnAddress) {
    return returnAddress == MessageBoxOkButtonFinalReturn ||
        returnAddress == MessageBoxCancelButtonFinalReturn;
}

bool isMessageBoxLabelSetRectReturn(DWORD returnAddress) {
    return returnAddress == MessageBoxFrameIconFinalReturn;
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

char* messageBoxOwnerFromLabel(char* control, DWORD& labelOffset) {
    DWORD vtable = 0;
    char* owner = control - MessageBoxFrameOffset;
    if (safeReadDword(owner, vtable) && isConfirmMessageBoxVtable(vtable)) {
        labelOffset = MessageBoxFrameOffset;
        return owner;
    }

    owner = control - MessageBoxFrameIconOffset;
    if (safeReadDword(owner, vtable) && isConfirmMessageBoxVtable(vtable)) {
        labelOffset = MessageBoxFrameIconOffset;
        return owner;
    }

    labelOffset = 0;
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

void scaleMessageBoxLabelRect(char* control, Rect& rect) {
    DWORD labelOffset = 0;
    char* owner = messageBoxOwnerFromLabel(control, labelOffset);
    if (!owner) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    if (labelOffset == MessageBoxFrameOffset) {
        rect.left *= TestScale;
        rect.top *= TestScale;
        rect.height *= TestScale;
        if (rect.width > root->width) {
            rect.width = root->width;
        }
        return;
    }

    const int centerX = rect.left + (rect.width / 2);
    const int scaledWidth = rect.width * TestScale;
    rect.left = centerX - (scaledWidth / 2);
    rect.top *= TestScale;
    rect.width = scaledWidth;
    rect.height *= TestScale;
}

void scaleMessageBoxFrameControl(char* owner, DWORD offset) {
    char* control = owner + offset;
    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    Rect* rect = reinterpret_cast<Rect*>(control + sizeof(DWORD));
    if (!hasUsefulRect(*root) || !hasUsefulRect(*rect)) {
        return;
    }

    if (offset == MessageBoxFrameOffset && rect->width >= root->width / 2) {
        return;
    }

    if (offset == MessageBoxFrameIconOffset && rect->width >= root->height / 4) {
        return;
    }

    callControlSetRect(control, scaledHeightRect(*rect));
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

void scalePopupDialogPanel(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || !isExpectedPopupVtable(vtable)) {
        return;
    }

    Rect* root = reinterpret_cast<Rect*>(owner + sizeof(DWORD));
    if (!hasUsefulRect(*root)) {
        return;
    }

    if (isSkillInfoTooltipVtable(vtable)) {
        scaleSkillInfoTooltipControls(owner);
        callControlSetRect(owner, scaledCenteredHeightRect(*root));
    }
    else {
        scalePanelControls(owner);
        callControlSetRect(owner, scaledRect(*root));
    }
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

    // FUN_00625C60 already derives the root width from measured text. Preserve
    // that width, but double its height to contain the scaled rows and button.
    if (returnAddress == StatusSummaryRootReturn) {
        *rect = doubledHeightCenteredRect(*rect);
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

void scaleMessageBoxLabelSetRect(void* control, DWORD* returnAddressSlot, DWORD* rectPointerSlot) {
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
    if (returnAddress == StatusSummaryIconReturn &&
        hasUsefulRect(*rect)) {
        // The game assigns active rows at Y=10, 47, 84, ... regardless of
        // message type. Keep the first row anchored and add a small amount of
        // spacing for each following visible row.
        const int row = (rect->top - StatusSummaryFirstRowTop) /
            StatusSummaryNativeRowSpacing;
        rect->top += row * StatusSummaryExtraIconSpacing;
        return;
    }

    if (isMessageBoxLabelSetRectReturn(returnAddress) &&
        hasUsefulRect(*rect)) {
        scaleMessageBoxLabelRect(static_cast<char*>(control), *rect);
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

    scaleMessageBoxFrameControl(owner, MessageBoxFrameOffset);
}

}
