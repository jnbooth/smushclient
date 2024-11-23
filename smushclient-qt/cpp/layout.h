#pragma once
#include <QtCore/QByteArray>
#include <QtCore/QMargins>
#include <QtGui/QBrush>
#include <QtGui/QColor>

struct OutputLayout
{
  QMargins margins;
  int16_t borderOffset;
  QColor borderColor;
  int16_t borderWidth;
  QBrush outsideFill;

  bool restore(const QByteArray &data);
  QByteArray save() const;
};
