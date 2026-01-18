#pragma once
#include "result.h"
#include <QtWidgets/QWidget>

namespace native {
SetBackgroundMaterialResult
setBackgroundMaterial(QWidget* widget, int material);

SetBackgroundMaterialResult
unsetBackgroundMaterial(QWidget* widget);
}
