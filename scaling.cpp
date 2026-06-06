#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Resolution scaling helpers

namespace {

void setGuiScaleState(int baseWidth, int baseHeight, int targetWidth, int targetHeight, bool hudScale) {
    g_guiBaseWidth = baseWidth;
    g_guiBaseHeight = baseHeight;
    g_guiTargetWidth = targetWidth;
    g_guiTargetHeight = targetHeight;
    g_hudScale = hudScale;
    g_classSelectionScale = false;
}

}

int screenWidth() {
    const int width = *ScreenWidth;
    return width > 0 ? width : BaseWidth;
}

int screenHeight() {
    const int height = *ScreenHeight;
    return height > 0 ? height : BaseHeight;
}

bool isBaseResolution() {
    const int width = screenWidth();
    const int height = screenHeight();

    return width == BaseWidth && height == BaseHeight;
}

int scaleCoordinate(int value, int target, int source) {
    if (source <= 0 || target <= 0) {
        return value;
    }

    return static_cast<int>((static_cast<long long>(value) * target) / source);
}

int integerScaleThatFits(int sourceWidth, int sourceHeight, int targetWidth, int targetHeight) {
    if (sourceWidth <= 0 || sourceHeight <= 0 || targetWidth <= 0 || targetHeight <= 0) {
        return 1;
    }

    int scaleX = targetWidth / sourceWidth;
    int scaleY = targetHeight / sourceHeight;
    int scale = scaleX < scaleY ? scaleX : scaleY;
    return scale > 1 ? scale : 1;
}

void patchCenteringConstants(int width, int height) {
    writeInt(0x0040AA65, width);
    writeInt(0x0040AA85, height);
    writeInt(0x0040B6C7, -width);
    writeInt(0x0040B6DA, -height);
    writeInt(0x0040BA6C, -width);
    writeInt(0x0040BA83, -height);
}

void setScale(int sourceWidth, int sourceHeight, int scaleBasisWidth, int scaleBasisHeight, bool updateCentering) {
    int targetBasisWidth = screenWidth();
    int targetBasisHeight = scaleCoordinate(scaleBasisHeight, targetBasisWidth, scaleBasisWidth);
    if (targetBasisHeight > screenHeight()) {
        targetBasisHeight = screenHeight();
        targetBasisWidth = scaleCoordinate(scaleBasisWidth, targetBasisHeight, scaleBasisHeight);
    }

    setGuiScaleState(
        sourceWidth,
        sourceHeight,
        scaleCoordinate(sourceWidth, targetBasisWidth, scaleBasisWidth),
        scaleCoordinate(sourceHeight, targetBasisHeight, scaleBasisHeight),
        false);

    if (updateCentering) {
        patchCenteringConstants(targetBasisWidth, targetBasisHeight);
    }
}

void setScreenScale(int sourceWidth, int sourceHeight, bool updateCentering) {
    setGuiScaleState(sourceWidth, sourceHeight, screenWidth(), screenHeight(), false);

    if (updateCentering) {
        patchCenteringConstants(g_guiTargetWidth, g_guiTargetHeight);
    }
}

void setHeightBasisScale(int sourceWidth, int sourceHeight, int heightBasis, bool updateCentering) {
    const int targetHeight = screenHeight();

    setGuiScaleState(
        sourceWidth,
        sourceHeight,
        scaleCoordinate(sourceWidth, targetHeight, heightBasis),
        scaleCoordinate(sourceHeight, targetHeight, heightBasis),
        false);

    if (updateCentering) {
        patchCenteringConstants(g_guiTargetWidth, g_guiTargetHeight);
    }
}

void setHudScale(int sourceWidth, int sourceHeight) {
    const int targetHeight = screenHeight();

    setGuiScaleState(
        sourceWidth,
        sourceHeight,
        scaleCoordinate(sourceWidth, targetHeight, sourceHeight),
        targetHeight,
        true);
}

void setIdentityScale(int sourceWidth, int sourceHeight, bool updateCentering) {
    setGuiScaleState(sourceWidth, sourceHeight, sourceWidth, sourceHeight, false);

    if (updateCentering) {
        patchCenteringConstants(sourceWidth, sourceHeight);
    }
}


}
