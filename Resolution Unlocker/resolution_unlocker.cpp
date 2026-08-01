#include <windows.h>

namespace {

constexpr uintptr_t VirtualMachinePointerAddress = 0x007A3A00;
constexpr uintptr_t AppManagerPointerAddress = 0x007A39FC;
constexpr uintptr_t PreviousAntiAliasAddress = 0x0078D440;
constexpr uintptr_t RequestedAntiAliasAddress = 0x007A6888;
constexpr uintptr_t VideoModeChangeRequestAddress = 0x007A3A2C;

constexpr size_t ClientAppOffset = 0x04;
constexpr size_t ResolutionButtonOffset = 0x08BC;
constexpr size_t AdvancedOptionsReinitOffset = 0x2330;

using SetButtonEnabledFn = void(__thiscall*)(void*, int);
using GetClientOptionsFn = void*(__thiscall*)(void*);

}

extern "C" void __cdecl setResolutionButtonAvailability(void* graphicsOptions) {
    if (!graphicsOptions) {
        return;
    }

    __try {
        void* virtualMachine =
            *reinterpret_cast<void* const volatile*>(VirtualMachinePointerAddress);
        void* resolutionButton =
            static_cast<unsigned char*>(graphicsOptions) + ResolutionButtonOffset;

        SetButtonEnabledFn setEnabled =
            reinterpret_cast<SetButtonEnabledFn>(0x00418DB0);
        setEnabled(resolutionButton, virtualMachine == nullptr ? 1 : 0);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl queueVideoModeResetForAntiAliasChange(void* advancedOptions) {
    if (!advancedOptions) {
        return;
    }

    __try {
        const int previousAntiAlias =
            *reinterpret_cast<volatile int*>(PreviousAntiAliasAddress);
        const int requestedAntiAlias =
            *reinterpret_cast<volatile int*>(RequestedAntiAliasAddress);
        if (previousAntiAlias == requestedAntiAlias) {
            return;
        }

        const int immediateReinit =
            *reinterpret_cast<const int*>(
                static_cast<const unsigned char*>(advancedOptions) +
                AdvancedOptionsReinitOffset);
        volatile int* videoModeChangeRequest =
            reinterpret_cast<volatile int*>(VideoModeChangeRequestAddress);
        if (immediateReinit != 0 || *videoModeChangeRequest != 0) {
            return;
        }

        void* appManager =
            *reinterpret_cast<void* const volatile*>(AppManagerPointerAddress);
        if (!appManager) {
            return;
        }

        void* clientApp =
            *reinterpret_cast<void**>(
                static_cast<unsigned char*>(appManager) + ClientAppOffset);
        if (!clientApp) {
            return;
        }

        GetClientOptionsFn getClientOptions =
            reinterpret_cast<GetClientOptionsFn>(0x005ED700);
        const unsigned int* clientOptions =
            static_cast<const unsigned int*>(getClientOptions(clientApp));
        if (!clientOptions) {
            return;
        }

        const bool fullscreen = (*clientOptions & 0x08) != 0;
        *videoModeChangeRequest = fullscreen ? 2 : 1;
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
