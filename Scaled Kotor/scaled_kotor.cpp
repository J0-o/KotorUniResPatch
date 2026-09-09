#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GameAPI/CExoIni.h"
#include "GameAPI/CExoString.h"
#include "GameAPI/GameVersion.h"
#include "resolution_scale.h"

namespace {

constexpr uintptr_t ScreenWidthAddress = 0x0078D1D4;
constexpr uintptr_t ScreenHeightAddress = 0x0078D1D8;
constexpr int DefaultScreenWidth = 800;
constexpr int DefaultScreenHeight = 600;
constexpr int BaseWidth = 640;
constexpr int BaseHeight = 480;
constexpr int MinimumContentScaleHeight = 1080;
constexpr int DefaultOverrideScaleFactor = 0;
constexpr int DefaultScaleFactor = 4;

char IniFile[] = "ScaledKotor.ini";
char IniSection[] = "Scaled Kotor";
char OverrideScaleFactorKey[] = "OverrideScaleFactor";
char ScaleFactorKey[] = "ScaleFactor";

UniversalScaleState scaleState = {
    DefaultScreenWidth,
    DefaultScreenHeight,
    BaseWidth,
    BaseHeight,
    1,
    1,
    1,
    1,
    0,
};

bool settingsLoaded = false;
bool scaleStateInitialized = false;
bool overrideScaleFactor = false;
double manualScaleFactor = DefaultScaleFactor;

bool pathBesideExe(const char* name, char* output, DWORD size) {
    const DWORD length = GetModuleFileNameA(nullptr, output, size);
    if (length == 0 || length >= size) {
        return false;
    }

    char* separator = strrchr(output, '\\');
    if (!separator ||
        static_cast<size_t>(separator + 1 - output) + strlen(name) + 1 > size) {
        return false;
    }

    strcpy(separator + 1, name);
    return true;
}

bool readIniInt(const char* path, char* key, int& value) {
    CExoIni ini;
    CExoString text;
    CExoString filename(const_cast<char*>(path));
    CExoString category(IniSection);
    CExoString entryKey(key);
    if (ini.ReadIniEntry(&text, &filename, &category, &entryKey) == 0) {
        return false;
    }

    char* raw = text.GetCStr();
    if (!raw) {
        return false;
    }

    value = atoi(raw);
    return true;
}

bool readIniDouble(const char* path, char* key, double& value) {
    CExoIni ini;
    CExoString text;
    CExoString filename(const_cast<char*>(path));
    CExoString category(IniSection);
    CExoString entryKey(key);
    if (ini.ReadIniEntry(&text, &filename, &category, &entryKey) == 0) {
        return false;
    }

    char* raw = text.GetCStr();
    if (!raw) {
        return false;
    }

    char* end = nullptr;
    const double parsed = strtod(raw, &end);
    if (end == raw || parsed <= 0.0) {
        return false;
    }

    value = parsed;
    return true;
}

void writeIniInt(const char* path, char* key, int value) {
    char buffer[16];
    sprintf(buffer, "%d", value);

    CExoIni ini;
    CExoString text(buffer);
    CExoString filename(const_cast<char*>(path));
    CExoString category(IniSection);
    CExoString entryKey(key);
    ini.WriteIniEntry(&text, &filename, &category, &entryKey);
}

void loadSettings() {
    if (settingsLoaded) {
        return;
    }
    settingsLoaded = true;

    char path[MAX_PATH];
    if (!pathBesideExe(IniFile, path, MAX_PATH)) {
        return;
    }

    int overrideValue = DefaultOverrideScaleFactor;
    if (!readIniInt(path, OverrideScaleFactorKey, overrideValue)) {
        writeIniInt(path, OverrideScaleFactorKey, DefaultOverrideScaleFactor);
    }

    double scaleValue = DefaultScaleFactor;
    if (!readIniDouble(path, ScaleFactorKey, scaleValue)) {
        writeIniInt(path, ScaleFactorKey, DefaultScaleFactor);
    }

    overrideScaleFactor = overrideValue != 0;
    manualScaleFactor = scaleValue;
}

int readPositiveInt(uintptr_t address, int fallback) {
    __try {
        const int value = *reinterpret_cast<volatile int*>(address);
        return value > 0 ? value : fallback;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return fallback;
    }
}

void updateScaleState() {
    loadSettings();

    const int screenWidth = readPositiveInt(ScreenWidthAddress, DefaultScreenWidth);
    const int screenHeight = readPositiveInt(ScreenHeightAddress, DefaultScreenHeight);
    if (scaleStateInitialized &&
        screenWidth == scaleState.screenWidth &&
        screenHeight == scaleState.screenHeight) {
        return;
    }

    int scaleNumerator = 0;
    int scaleDenominator = 0;
    if (static_cast<long long>(screenHeight) * BaseWidth <=
        static_cast<long long>(screenWidth) * BaseHeight) {
        scaleNumerator = screenHeight;
        scaleDenominator = BaseHeight;
    }
    else {
        scaleNumerator = screenWidth;
        scaleDenominator = BaseWidth;
    }

    const int contentScalingEnabled =
        screenHeight >= MinimumContentScaleHeight ? 1 : 0;
    int contentScaleNumerator = 1;
    int contentScaleDenominator = 1;
    if (contentScalingEnabled && overrideScaleFactor) {
        contentScaleNumerator =
            static_cast<int>(manualScaleFactor * 1000.0 + 0.5);
        contentScaleDenominator = 1000;
    }
    else if (contentScalingEnabled) {
        contentScaleNumerator = scaleNumerator;
        contentScaleDenominator = scaleDenominator;
    }

    const int uiWidth = static_cast<int>(
        (static_cast<long long>(BaseWidth) * scaleNumerator) / scaleDenominator);
    const int uiHeight = static_cast<int>(
        (static_cast<long long>(BaseHeight) * scaleNumerator) / scaleDenominator);

    scaleState = {
        screenWidth,
        screenHeight,
        uiWidth,
        uiHeight,
        scaleNumerator,
        scaleDenominator,
        contentScaleNumerator,
        contentScaleDenominator,
        contentScalingEnabled,
    };
    scaleStateInitialized = true;
}

}

extern "C" const UniversalScaleState* __cdecl getUniversalScaleState() {
    updateScaleState();
    return &scaleState;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(reserved);

    if (reason == DLL_PROCESS_ATTACH) {
        GameVersion::Initialize();
    }
    else if (reason == DLL_PROCESS_DETACH) {
        GameVersion::Reset();
    }

    return TRUE;
}
