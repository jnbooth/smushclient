#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "rust/cxx.h"

namespace rust::smushclientqtlib1::qstring {
QString&
append(QString& s, QChar ch)
{
  return s.append(ch);
}

QChar
at(const QString& s, rust::isize position)
{
  return s.at(static_cast<qsizetype>(position));
}

size_t
indexOf(const QString& s, QChar ch)
{
  return s.indexOf(ch);
}

QStringList
split(const QString& s,
      QChar sep,
      Qt::SplitBehaviorFlags behavior,
      Qt::CaseSensitivity cs)
{
  return s.split(sep, behavior, cs);
}
} // namespace rust::smushclientqtlib1
