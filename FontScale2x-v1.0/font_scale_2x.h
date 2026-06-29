#pragma once

#include <windows.h>

namespace FontScale2x {

void scaleFontBeforeTextOut(void* font);
void scaleGuiStringBeforeDraw(void* guiString);

}
