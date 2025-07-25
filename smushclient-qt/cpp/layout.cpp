#include "layout.h"
#include <QtCore/QDataStream>
#include <QtCore/QIODevice>
#include <QtCore/QList>
#include <QtCore/QVariant>

QDataStream &operator>>(QDataStream &stream, OutputLayout &layout) {
  return stream >> layout.margins >> layout.borderOffset >>
         layout.borderColor >> layout.borderWidth >> layout.outsideFill;
}

QByteArray OutputLayout::save() const {
  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);
  stream << margins << borderOffset << borderColor << borderWidth
         << outsideFill;
  return data;
}

bool OutputLayout::restore(const QByteArray &data) {
  if (data.isEmpty())
    return false;

  QDataStream stream(data);
  stream >> margins >> borderOffset >> borderColor >> borderWidth >>
      outsideFill;
  return stream.status() == QDataStream::Status::Ok;
}
