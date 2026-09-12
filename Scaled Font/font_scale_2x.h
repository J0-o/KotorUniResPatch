#pragma once

#include <windows.h>

namespace FontScale2x {

void scaleLoadedTextureMetadata(void* textureMetadata);
void scaleResetGuiStringFont(void* guiString);
void refreshResolutionDependentUi();

}
