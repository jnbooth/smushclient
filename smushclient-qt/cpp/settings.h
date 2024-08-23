#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore/QSettings>

typedef struct RecentFileResult
{
  bool changed;
  QStringList recentFiles;
} RecentFileResult;

class Settings
{
public:
  explicit Settings();

  QStringList recentFiles() const;
  RecentFileResult addRecentFile(const QString &path);
  RecentFileResult removeRecentFile(const QString &path);

private:
  QSettings store;
};

#endif // SETTINGS_H
