#include "settings.h"

#define KEY_RECENT_FILES "recent"
#define MAX_RECENT_FILES 5

Settings::Settings() : store() {}

// Recent files

QStringList Settings::recentFiles() const
{
  QStringList recent = store.value(KEY_RECENT_FILES).toStringList();
  // Should never happen, but just in case.
  if (recent.size() > MAX_RECENT_FILES) [[unlikely]]
    recent.resize(MAX_RECENT_FILES);
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
  else if (recent.size() == MAX_RECENT_FILES)
    recent.removeLast();

  recent.prepend(path);
  store.setValue(KEY_RECENT_FILES, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}

RecentFileResult Settings::removeRecentFile(const QString &path)
{
  QStringList recent = recentFiles();
  qsizetype index = recent.indexOf(path);
  if (index == -1)
    return RecentFileResult{.changed = false, .recentFiles = recent};

  recent.remove(index);
  store.setValue(KEY_RECENT_FILES, recent);
  return RecentFileResult{.changed = true, .recentFiles = recent};
}
