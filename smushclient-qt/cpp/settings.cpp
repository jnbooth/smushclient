#include "settings.h"
#include <QtCore/QDir>
#include <QtCore/QUuid>

static const QString recentFilesKey = QStringLiteral("recent");
constexpr qsizetype recentFilesMax = 5;

// Private utilities

QString makePathRelative(const QString &filePath)
{
  const QString cwd = QDir::currentPath();
  if (!filePath.startsWith(cwd))
    return filePath;
  const qsizetype cwdSize = cwd.size() + 1;
  return filePath.sliced(cwdSize, filePath.size() - cwdSize);
}

// Public methods

Settings::Settings() : store() {}

QStringList Settings::recentFiles() const
{
  QStringList recent = store.value(recentFilesKey).toStringList();
  // Should never happen, but just in case.
  if (recent.size() > recentFilesMax)
    recent.resize(recentFilesMax);
  return recent;
}

RecentFileResult Settings::addRecentFile(const QString &path)
{
  QString relativePath = makePathRelative(path);
  QStringList recent = recentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == 0)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  if (index != -1)
    recent.remove(index);
  else if (recent.size() == recentFilesMax)
    recent.removeLast();

  recent.prepend(relativePath);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}

RecentFileResult Settings::removeRecentFile(const QString &path)
{
  QString relativePath = makePathRelative(path);
  QStringList recent = recentFiles();
  qsizetype index = recent.indexOf(relativePath);
  if (index == -1)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  recent.remove(index);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}
