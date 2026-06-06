#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Safe memory probes

bool safeReadDword(const void* address, DWORD& value) {
    __try {
        value = *static_cast<const DWORD*>(address);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0;
        return false;
    }
}

bool memoryContainsText(const void* address, int size, const char* needle) {
    if (!address || size <= 0 || !needle || !*needle) {
        return false;
    }

    __try {
        const char* bytes = static_cast<const char*>(address);
        for (int i = 0; i < size; ++i) {
            int j = 0;
            while (needle[j] && i + j < size && bytes[i + j] == needle[j]) {
                ++j;
            }
            if (!needle[j]) {
                return true;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return false;
}


}
