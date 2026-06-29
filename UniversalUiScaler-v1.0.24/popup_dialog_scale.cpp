#include "popup_dialog_scale.h"

namespace PopupDialogScale {

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

constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;
constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;

constexpr DWORD StatusSummaryIconReturn = 0x0062615C;
constexpr DWORD StatusSummaryTextReturn = 0x0062619A;
constexpr DWORD StatusSummaryTextAdjustReturn = 0x006261F4;
constexpr DWORD StatusSummaryRootReturn = 0x006262B7;
constexpr DWORD StatusSummaryLowerReturn = 0x00626301;
constexpr DWORD MessageBoxFixMessageLabelStart = 0x006253A0;
constexpr DWORD MessageBoxFixMessageLabelEnd = 0x006258F0;
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

int screenWidth() {
    int width = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenWidthAddress), width) || width <= 0) {
        return BaseWidth;
    }

    return width;
}

int screenHeight() {
    int height = 0;
    if (!safeReadInt(reinterpret_cast<const void*>(ScreenHeightAddress), height) || height <= 0) {
        return BaseHeight;
    }

    return height;
}

int scaleValue(int value, int target, int source) {
    if (target <= 0 || source <= 0) {
        return value;
    }

    return static_cast<int>((static_cast<long long>(value) * target) / source);
}

int menuWidth() {
    int width = scaleValue(BaseWidth, screenHeight(), BaseHeight);
    if (width > screenWidth()) {
        width = screenWidth();
    }

    return width;
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

Rect scaledHeightRect(const Rect& rect) {
    const int height = screenHeight();
    return {
        scaleValue(rect.left, height, BaseHeight),
        scaleValue(rect.top, height, BaseHeight),
        scaleValue(rect.width, height, BaseHeight),
        scaleValue(rect.height, height, BaseHeight),
    };
}

Rect scaledCenteredHeightRect(const Rect& rect) {
    const int height = screenHeight();
    const int centerX = rect.left + (rect.width / 2);
    const int centerY = rect.top + (rect.height / 2);
    const int scaledWidth = scaleValue(rect.width, height, BaseHeight);
    const int scaledHeight = scaleValue(rect.height, height, BaseHeight);

    return {
        centerX - (scaledWidth / 2),
        centerY - (scaledHeight / 2),
        scaledWidth,
        scaledHeight,
    };
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
    return returnAddress >= MessageBoxFixMessageLabelStart &&
        returnAddress < MessageBoxFixMessageLabelEnd;
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
        rect.left = scaleValue(rect.left, screenHeight(), BaseHeight);
        rect.top = scaleValue(rect.top, screenHeight(), BaseHeight);
        rect.height = scaleValue(rect.height, screenHeight(), BaseHeight);
        if (rect.width > root->width) {
            rect.width = root->width;
        }
        return;
    }

    const int centerX = rect.left + (rect.width / 2);
    const int scaledWidth = scaleValue(rect.width, screenHeight(), BaseHeight);
    rect.left = centerX - (scaledWidth / 2);
    rect.top = scaleValue(rect.top, screenHeight(), BaseHeight);
    rect.width = scaledWidth;
    rect.height = scaleValue(rect.height, screenHeight(), BaseHeight);
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
    return returnAddress == StatusSummaryIconReturn ||
        returnAddress == StatusSummaryTextReturn ||
        returnAddress == StatusSummaryTextAdjustReturn ||
        returnAddress == StatusSummaryRootReturn ||
        returnAddress == StatusSummaryLowerReturn;
}

}

void scalePopupDialogPanel(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner || (screenWidth() == BaseWidth && screenHeight() == BaseHeight)) {
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

    if (!returnAddressSlot || !rectPointerSlot || (screenWidth() == BaseWidth && screenHeight() == BaseHeight)) {
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

    if (returnAddress == StatusSummaryRootReturn) {
        *rect = scaledCenteredHeightRect(*rect);
    }
    else {
        *rect = scaledHeightRect(*rect);
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
    if (isMessageBoxButtonSetExtentReturn(returnAddress) &&
        hasUsefulRect(*rect) &&
        (screenWidth() != BaseWidth || screenHeight() != BaseHeight)) {
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
    if (isMessageBoxButtonSetExtentReturn(returnAddress) &&
        hasUsefulRect(*rect) &&
        (screenWidth() != BaseWidth || screenHeight() != BaseHeight)) {
        scaleMessageBoxLabelRect(static_cast<char*>(control), *rect);
    }
}

void scaleMessageBoxAfterFix(void* ownerPtr) {
    char* owner = static_cast<char*>(ownerPtr);
    if (!owner || (screenWidth() == BaseWidth && screenHeight() == BaseHeight)) {
        return;
    }

    DWORD vtable = 0;
    if (!safeReadDword(owner, vtable) || !isConfirmMessageBoxVtable(vtable)) {
        return;
    }

    scaleMessageBoxFrameControl(owner, MessageBoxFrameOffset);
    scaleMessageBoxFrameControl(owner, MessageBoxFrameIconOffset);
}

}
