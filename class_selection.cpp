#include "widescreen_ui_scale.h"

namespace WidescreenUiScale {

// Class selection picker layout fix

struct GuiEdit {
    Rect source;
    Rect target;
    int pair;
    bool isModel;
};

static const GuiEdit g_guiEdits[] = {
    { { 55, 118, 95, 213 },  { 139, 118, 189, 672 },  0, false },
    { { 58, 121, 89, 207 },  { 146, 285, 175, 441 },  0, true },
    { { 152, 128, 75, 193 }, { 342, 128, 169, 651 },  1, false },
    { { 155, 131, 69, 187 }, { 349, 295, 155, 421 },  1, true },
    { { 239, 128, 75, 193 }, { 538, 128, 169, 651 },  2, false },
    { { 242, 131, 69, 187 }, { 545, 295, 155, 421 },  2, true },
    { { 325, 128, 75, 193 }, { 731, 128, 169, 651 },  3, false },
    { { 328, 131, 69, 187 }, { 738, 295, 155, 421 },  3, true },
    { { 413, 128, 75, 193 }, { 929, 128, 169, 651 },  4, false },
    { { 416, 131, 69, 187 }, { 936, 295, 155, 421 },  4, true },
    { { 500, 128, 75, 193 }, { 1125, 128, 169, 651 }, 5, false },
    { { 503, 131, 69, 187 }, { 1132, 295, 155, 421 }, 5, true }
};

constexpr int ClassSelectionDesignWidth = 1440;
constexpr int ClassSelectionDesignHeight = 1080;

bool rectEquals(const Rect& left, const Rect& right) {
    return left.left == right.left &&
        left.top == right.top &&
        left.width == right.width &&
        left.height == right.height;
}

Rect scaledRect(const Rect& rect) {
    Rect scaled = {
        scaleCoordinate(rect.left, g_guiTargetWidth, g_guiBaseWidth),
        scaleCoordinate(rect.top, g_guiTargetHeight, g_guiBaseHeight),
        scaleCoordinate(rect.width, g_guiTargetWidth, g_guiBaseWidth),
        scaleCoordinate(rect.height, g_guiTargetHeight, g_guiBaseHeight)
    };
    return scaled;
}

const GuiEdit* findPairedClassSelectionEdit(const GuiEdit& edit) {
    for (size_t i = 0; i < sizeof(g_guiEdits) / sizeof(g_guiEdits[0]); ++i) {
        const GuiEdit& candidate = g_guiEdits[i];
        if (candidate.pair == edit.pair && candidate.isModel != edit.isModel) {
            return &candidate;
        }
    }

    return nullptr;
}

Rect classSelectionTargetRect(const GuiEdit& edit) {
    Rect target = {
        scaleCoordinate(edit.target.left, g_guiTargetWidth, ClassSelectionDesignWidth),
        edit.target.top,
        scaleCoordinate(edit.target.width, g_guiTargetWidth, ClassSelectionDesignWidth),
        scaleCoordinate(edit.target.height, g_guiTargetHeight, ClassSelectionDesignHeight)
    };

    if (edit.isModel) {
        target.top = (g_guiTargetHeight - target.height) / 2;
    }
    else {
        const GuiEdit* model = findPairedClassSelectionEdit(edit);
        if (model) {
            const int modelHeight = scaleCoordinate(model->target.height, g_guiTargetHeight, ClassSelectionDesignHeight);
            const int modelTop = (g_guiTargetHeight - modelHeight) / 2;
            const int bottomPadding = (edit.source.top + edit.source.height) - (model->source.top + model->source.height);
            const int scaledBottomPadding = scaleCoordinate(bottomPadding, g_guiTargetHeight, g_guiBaseHeight);
            target.height = (modelTop + modelHeight + scaledBottomPadding) - target.top;
        }

        if (target.height < 1) {
            target.height = 1;
        }
    }

    return target;
}

const GuiEdit* findGuiEditBySourceRect(const Rect& rect) {
    if (!g_classSelectionScale) {
        return nullptr;
    }

    for (size_t i = 0; i < sizeof(g_guiEdits) / sizeof(g_guiEdits[0]); ++i) {
        const GuiEdit& edit = g_guiEdits[i];
        if (rectEquals(rect, edit.source) || rectEquals(rect, scaledRect(edit.source))) {
            return &edit;
        }
    }

    return nullptr;
}

bool isPackedClassSelectionPickerRect(const Rect& rect) {
    return rect.top >= 260 && rect.top <= 300 &&
        rect.height >= 420 && rect.height <= 490 &&
        rect.width >= 150 && rect.width <= 220;
}

const GuiEdit* findNearestClassSelectionPickerEdit(const Rect& rect) {
    if (!g_classSelectionScale) {
        return nullptr;
    }

    int nearestDistance = 0x7FFFFFFF;
    const GuiEdit* nearest = nullptr;
    const int center = rect.left + (rect.width / 2);
    const bool likelyInner = rect.width < 180;

    for (size_t i = 0; i < sizeof(g_guiEdits) / sizeof(g_guiEdits[0]); ++i) {
        const GuiEdit& edit = g_guiEdits[i];
        if (edit.isModel != likelyInner) {
            continue;
        }

        const Rect target = classSelectionTargetRect(edit);
        const int scaledCenter = target.left + (target.width / 2);
        int distance = center - scaledCenter;
        if (distance < 0) {
            distance = -distance;
        }

        if (distance < nearestDistance) {
            nearestDistance = distance;
            nearest = &edit;
        }
    }

    return nearest;
}

bool isClassSelectionPickerRect(const Rect& rect) {
    return findGuiEditBySourceRect(rect) != nullptr ||
        isPackedClassSelectionPickerRect(rect);
}

bool applyGuiExtentEdit(Rect* rect) {
    if (!rect) {
        return false;
    }

    const GuiEdit* edit = findGuiEditBySourceRect(*rect);
    if (!edit && isPackedClassSelectionPickerRect(*rect)) {
        edit = findNearestClassSelectionPickerEdit(*rect);
    }

    if (!edit) {
        return false;
    }

    *rect = classSelectionTargetRect(*edit);
    return true;
}


}
