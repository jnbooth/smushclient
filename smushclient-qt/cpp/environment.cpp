#include "environment.h"
#include <QtCore/QDir>

QString makePathRelative(const QString &filePath)
{
  return makePathRelative(filePath, QDir::currentPath());
}

QString makePathRelative(const QString &filePath, const QString &relativeTo)
{
  if (!filePath.startsWith(relativeTo))
    return filePath;
  const qsizetype relativeToSize = relativeTo.size() + 1;
  return filePath.sliced(relativeToSize, filePath.size() - relativeToSize);
}
