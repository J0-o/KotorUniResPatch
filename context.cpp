#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Context matching helpers

bool gffContextContains(DWORD gffContext, const char* token) {
    DWORD words[8] = {};
    for (int i = 0; i < 8; ++i) {
        safeReadDword(reinterpret_cast<const void*>(gffContext + (i * sizeof(DWORD))), words[i]);
    }

    return memoryContainsText(reinterpret_cast<const void*>(gffContext), 512, token) ||
        memoryContainsText(reinterpret_cast<const void*>(words[4]), 512, token) ||
        memoryContainsText(reinterpret_cast<const void*>(words[5]), 512, token);
}

bool callContextContains(DWORD* stack, const char* token) {
    if (!stack) {
        return false;
    }

    DWORD stack0 = 0;
    DWORD stack4 = 0;
    safeReadDword(stack + 0, stack0);
    safeReadDword(stack + 1, stack4);

    return memoryContainsText(reinterpret_cast<const void*>(stack0), 512, token) ||
        memoryContainsText(reinterpret_cast<const void*>(stack4), 512, token);
}


}
