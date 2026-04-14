#pragma once
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QMargins>
#include <QtGui/QBrush>
#include <QtGui/QColor>

struct OutputLayout
{
  QMargins margins;
  int16_t borderOffset = 0;
  QColor borderColor;
  int16_t borderWidth = 0;
  QBrush outsideFill;

  bool restore(const QByteArray& data);
  QByteArray save() const;
};

QDataStream&
operator>>(QDataStream& stream, OutputLayout& layout);
