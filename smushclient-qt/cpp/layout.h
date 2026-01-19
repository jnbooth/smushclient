#pragma once
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
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

  bool restore(const QByteArray& data);
  QByteArray save() const;
};

inline QDataStream&
operator>>(QDataStream& stream, OutputLayout& layout)
{
  return stream >> layout.margins >> layout.borderOffset >>
         layout.borderColor >> layout.borderWidth >> layout.outsideFill;
}
