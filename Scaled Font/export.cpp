#include "font_scale_2x.h"

extern "C" void __cdecl scaleLoadedTextureMetadata(void* textureMetadata) {
    __try {
        FontScale2x::scaleLoadedTextureMetadata(textureMetadata);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleResetGuiStringFont(void* guiString) {
    __try {
        FontScale2x::scaleResetGuiStringFont(guiString);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl refreshResolutionDependentUi() {
    __try {
        FontScale2x::refreshResolutionDependentUi();
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}
