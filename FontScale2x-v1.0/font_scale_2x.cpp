#include "font_scale_2x.h"

namespace FontScale2x {

namespace {

constexpr float Scale = 2.0f;
constexpr DWORD FontInfoOffset = 0x18;
constexpr DWORD FontHeightOffset = 0x04;
constexpr DWORD BaselineHeightOffset = 0x08;
constexpr DWORD TextureWidthOffset = 0x0C;
constexpr DWORD SpacingROffset = 0x10;
constexpr DWORD SpacingBOffset = 0x14;
constexpr DWORD GuiStringFontTextureOffset = 0x18;
constexpr DWORD SafePointerGetFontInfoVtableOffset = 0x38;
constexpr int MaxTrackedFontInfos = 64;

void* g_scaledFontInfos[MaxTrackedFontInfos] = {};
int g_scaledFontInfoCount = 0;

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

bool safeReadFloat(const void* address, float& value) {
    __try {
        value = *reinterpret_cast<const float*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0.0f;
        return false;
    }
}

bool wasScaled(void* fontInfo) {
    for (int i = 0; i < g_scaledFontInfoCount; ++i) {
        if (g_scaledFontInfos[i] == fontInfo) {
            return true;
        }
    }

    return false;
}

bool rememberScaled(void* fontInfo) {
    if (g_scaledFontInfoCount >= MaxTrackedFontInfos) {
        return false;
    }

    g_scaledFontInfos[g_scaledFontInfoCount++] = fontInfo;
    return true;
}

void scaleFloat(char* base, DWORD offset) {
    float* value = reinterpret_cast<float*>(base + offset);
    *value *= Scale;
}

bool hasSaneFontMetrics(char* fontInfo) {
    float fontHeight = 0.0f;
    float baselineHeight = 0.0f;
    float textureWidth = 0.0f;
    return safeReadFloat(fontInfo + FontHeightOffset, fontHeight) &&
        safeReadFloat(fontInfo + BaselineHeightOffset, baselineHeight) &&
        safeReadFloat(fontInfo + TextureWidthOffset, textureWidth) &&
        fontHeight > 0.0f && fontHeight < 512.0f &&
        baselineHeight > 0.0f && baselineHeight < 512.0f &&
        textureWidth > 0.0f && textureWidth < 4096.0f;
}

void scaleFontInfo(void* fontInfoPtr) {
    if (!fontInfoPtr || wasScaled(fontInfoPtr)) {
        return;
    }

    __try {
        char* fontInfo = static_cast<char*>(fontInfoPtr);
        if (!hasSaneFontMetrics(fontInfo) || !rememberScaled(fontInfoPtr)) {
            return;
        }

        scaleFloat(fontInfo, FontHeightOffset);
        scaleFloat(fontInfo, BaselineHeightOffset);
        scaleFloat(fontInfo, TextureWidthOffset);
        scaleFloat(fontInfo, SpacingROffset);
        scaleFloat(fontInfo, SpacingBOffset);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

void* fontInfoFromGuiString(void* guiStringPtr) {
    char* guiString = static_cast<char*>(guiStringPtr);
    DWORD fontTextureSafePointer = 0;
    if (!safeReadDword(guiString + GuiStringFontTextureOffset, fontTextureSafePointer) ||
        fontTextureSafePointer == 0) {
        return nullptr;
    }

    __try {
        DWORD vtable = *reinterpret_cast<DWORD*>(fontTextureSafePointer);
        DWORD getFontInfo = *reinterpret_cast<DWORD*>(vtable + SafePointerGetFontInfoVtableOffset);
        if (getFontInfo == 0) {
            return nullptr;
        }

        typedef void* (__thiscall *GetFontInfoFn)(void*);
        return reinterpret_cast<GetFontInfoFn>(getFontInfo)(reinterpret_cast<void*>(fontTextureSafePointer));
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

}

void scaleFontBeforeTextOut(void* font) {
    if (!font) {
        return;
    }

    DWORD fontInfo = 0;
    if (safeReadDword(static_cast<char*>(font) + FontInfoOffset, fontInfo) && fontInfo != 0) {
        scaleFontInfo(reinterpret_cast<void*>(fontInfo));
    }
}

void scaleGuiStringBeforeDraw(void* guiString) {
    if (!guiString) {
        return;
    }

    scaleFontInfo(fontInfoFromGuiString(guiString));
}

}
