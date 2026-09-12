#include "menu_background_shift_test.h"

extern "C" void __cdecl center800x600backDynamic(void* parent) {
    __try {
        MenuBackgroundShiftTest::centerMenuBackgroundDynamic(parent);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}
