#pragma once
#include "../scriptenums.h"
#include <QtGui/QPixmap>

namespace blend {
bool
image(QPixmap& target,
      const QPixmap& source,
      const QPointF& origin,
      BlendMode mode,
      qreal opacity,
      const QRectF& sourceRect);
} // namespace blend
