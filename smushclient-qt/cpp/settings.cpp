#include "settings.h"

static const QString recentFilesKey = QStringLiteral("recent");
static constexpr qsizetype recentFilesMax = 5;

Settings::Settings() : store() {}

// Recent files

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
  QStringList recent = recentFiles();
  qsizetype index = recent.indexOf(path);
  if (index == 0)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  if (index != -1)
    recent.remove(index);
  else if (recent.size() == recentFilesMax)
    recent.removeLast();

  recent.prepend(path);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}

RecentFileResult Settings::removeRecentFile(const QString &path)
{
  QStringList recent = recentFiles();
  qsizetype index = recent.indexOf(path);
  if (index == -1)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  recent.remove(index);
  store.setValue(recentFilesKey, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}
