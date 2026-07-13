#include "menu_background_shift_test.h"

namespace MenuBackgroundShiftTest {

namespace {

constexpr DWORD BackgroundChildOffset = 0x5C;
constexpr DWORD BackgroundLeftOffset = 0x04;
constexpr DWORD BackgroundWidthOffset = 0x0C;
constexpr DWORD BackgroundNameOffset = 0x54;
constexpr DWORD BackgroundChildVtable = 0x0073E338;
constexpr DWORD ScreenWidthAddress = 0x0078D1D4;
constexpr DWORD ScreenHeightAddress = 0x0078D1D8;
constexpr int BaseWidth = 800;
constexpr int BaseHeight = 600;
constexpr int ImageWidth = 4096;
constexpr int CenterSafeWidth = 2048;
constexpr int SafeAspectWidth = 4;
constexpr int SafeAspectHeight = 3;
constexpr char MenuBackgroundName[] = "800x600back";
constexpr char ComputerBackgroundName[] = "800x600comp0";
constexpr char PazaakBackgroundName[] = "800x600pazaak";
constexpr char LoadBackgroundName[] = "800x600load";
constexpr char MapBackgroundName[] = "800x600map";
constexpr char StoreBackgroundName[] = "800x600store";
constexpr char ComputerAltBackgroundName[] = "800x600comp1";

bool stringEquals(const char* actual, const char* expected) {
    for (;;) {
        if (*actual != *expected) {
            return false;
        }

        if (*actual == '\0') {
            return true;
        }

        ++actual;
        ++expected;
    }
}

int readPositiveInt(DWORD address, int fallback) {
    int value = 0;
    __try {
        value = *reinterpret_cast<int*>(address);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        value = 0;
    }

    return value > 0 ? value : fallback;
}

int scaleWidthFromHeight(int height) {
    constexpr int numerator = ImageWidth * SafeAspectWidth;
    constexpr int denominator = CenterSafeWidth * SafeAspectHeight;
    return static_cast<int>((static_cast<long long>(height) * numerator + denominator / 2) / denominator);
}

void centerControlFor4096Background(char* control, int screenWidth, int screenHeight) {
    int backgroundWidth = scaleWidthFromHeight(screenHeight);
    int backgroundLeft = (screenWidth - backgroundWidth) / 2;

    *reinterpret_cast<int*>(control + BackgroundLeftOffset) = backgroundLeft;
    *reinterpret_cast<int*>(control + BackgroundWidthOffset) = backgroundWidth;
}

bool isKnownBackgroundName(const char* name) {
    return stringEquals(name, MenuBackgroundName) ||
        stringEquals(name, ComputerBackgroundName) ||
        stringEquals(name, PazaakBackgroundName) ||
        stringEquals(name, LoadBackgroundName) ||
        stringEquals(name, MapBackgroundName) ||
        stringEquals(name, StoreBackgroundName) ||
        stringEquals(name, ComputerAltBackgroundName);
}

}

void centerMenuBackgroundDynamic(void* parent) {
    if (!parent) {
        return;
    }

    __try {
        char* child = static_cast<char*>(parent) + BackgroundChildOffset;
        DWORD vtable = *reinterpret_cast<DWORD*>(child);
        if (vtable != BackgroundChildVtable) {
            return;
        }

        const char* name = child + BackgroundNameOffset;
        if (!isKnownBackgroundName(name)) {
            return;
        }

        int screenWidth = readPositiveInt(ScreenWidthAddress, BaseWidth);
        int screenHeight = readPositiveInt(ScreenHeightAddress, BaseHeight);
        centerControlFor4096Background(child, screenWidth, screenHeight);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

}
