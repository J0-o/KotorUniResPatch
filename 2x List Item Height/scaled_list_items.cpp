#include <windows.h>

#include "../Common/ResolutionScale.h"

namespace {

constexpr DWORD ItemVisualHeightAddress = 0x006B527F;
constexpr DWORD QuantityHeightAddress = 0x006B5332;
constexpr DWORD QuantityWidthShortAddress = 0x006B5339;
constexpr DWORD QuantityWidthLongAddress = 0x006B533C;
constexpr DWORD QuantityTopAddress = 0x006B5351;
constexpr DWORD SkillVisualHeightAddress = 0x006AB8EF;

bool writeMemory(void* address, const void* value, size_t size) {
    __try {
        DWORD oldProtect = 0;
        if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
            return false;
        }

        CopyMemory(address, value, size);
        FlushInstructionCache(GetCurrentProcess(), address, size);

        DWORD ignored = 0;
        VirtualProtect(address, size, oldProtect, &ignored);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

void writeInt(DWORD address, int value) {
    writeMemory(reinterpret_cast<void*>(address), &value, sizeof(value));
}

void writeByte(DWORD address, BYTE value) {
    writeMemory(reinterpret_cast<void*>(address), &value, sizeof(value));
}

int adjustedValue(int vanillaValue, const UniversalScaleState& scale) {
    return scaleTwoXValue(vanillaValue, scale);
}

BYTE adjustedByte(int vanillaValue, const UniversalScaleState& scale) {
    const int value = adjustedValue(vanillaValue, scale);
    return static_cast<BYTE>(value < 0 ? 0 : (value > 255 ? 255 : value));
}

void writeStackValue(void* slot, int vanillaValue,
                     const UniversalScaleState& scale) {
    if (slot) {
        *static_cast<int*>(slot) = adjustedValue(vanillaValue, scale);
    }
}

void patchItemVisualConstants(const UniversalScaleState& scale) {
    writeInt(ItemVisualHeightAddress, adjustedValue(0x38, scale));
    writeInt(QuantityHeightAddress, adjustedValue(0x13, scale));
    writeByte(QuantityWidthShortAddress, adjustedByte(0x15, scale));
    writeByte(QuantityWidthLongAddress, adjustedByte(0x15, scale));
    writeByte(QuantityTopAddress, adjustedByte(0x25, scale));
}

const UniversalScaleState* getScale() {
    return ResolutionScale::get();
}

void __cdecl refreshPatchedListConstants() {
    const UniversalScaleState* scale = getScale();
    if (!scale) {
        return;
    }
    patchItemVisualConstants(*scale);
    writeInt(SkillVisualHeightAddress, adjustedValue(0x2A, *scale));
}

}

extern "C" void __cdecl setInventoryItemRowHeight(void* heightSlot) {
    const UniversalScaleState* scale = getScale();
    if (!scale) {
        return;
    }

    writeStackValue(heightSlot, 0x38, *scale);
    patchItemVisualConstants(*scale);
}

extern "C" void __cdecl setSkillItemRowHeight(void* heightSlot) {
    const UniversalScaleState* scale = getScale();
    if (!scale) {
        return;
    }

    writeStackValue(heightSlot, 0x2A, *scale);
    writeInt(SkillVisualHeightAddress, adjustedValue(0x2A, *scale));
}

extern "C" void __cdecl setFeatGroupRowHeight(void* heightSlot) {
    const UniversalScaleState* scale = getScale();
    if (!scale) {
        return;
    }

    writeStackValue(heightSlot, 0x28, *scale);
}

extern "C" void __cdecl setFeatStoredExtent(void* sourceRect,
                                              void* destinationOwner) {
    const UniversalScaleState* scale = getScale();
    if (!sourceRect || !destinationOwner || !scale) {
        return;
    }

    __try {
        int* source = static_cast<int*>(sourceRect);
        int* destination = reinterpret_cast<int*>(
            static_cast<char*>(destinationOwner) + 0x04);
        destination[0] = source[0];
        destination[1] = source[1];

        char* widthCursor = reinterpret_cast<char*>(destination + 2);
        destination[2] =
            *reinterpret_cast<int*>(widthCursor + 0x2A4) +
            *reinterpret_cast<int*>(widthCursor + 0x2AC);
        destination[3] = adjustedValue(0x28, *scale);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl setGenericListRowHeight(void* sourceRect,
                                                  void* destinationRect) {
    const UniversalScaleState* scale = getScale();
    if (!sourceRect || !destinationRect || !scale) {
        return;
    }

    __try {
        const int sourceHeight = *reinterpret_cast<int*>(
            static_cast<char*>(sourceRect) + 0x0C);
        *reinterpret_cast<int*>(static_cast<char*>(destinationRect) + 0x0C) =
            adjustedValue(sourceHeight, *scale);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

extern "C" void __cdecl refreshResolutionDependentUi() {
    refreshPatchedListConstants();
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reserved);
    if (reason == DLL_PROCESS_ATTACH) {
        ResolutionScale::subscribe(refreshResolutionDependentUi);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        ResolutionScale::unsubscribe(refreshResolutionDependentUi);
    }
    return TRUE;
}
