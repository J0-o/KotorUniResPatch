#include "font_scale_2x.h"

extern "C" void __cdecl scaleFontBeforeTextOut(void* font) {
    __try {
        FontScale2x::scaleFontBeforeTextOut(font);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl scaleGuiStringBeforeDraw(void* guiString) {
    __try {
        FontScale2x::scaleGuiStringBeforeDraw(guiString);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reason);
    UNREFERENCED_PARAMETER(reserved);
    return TRUE;
}
