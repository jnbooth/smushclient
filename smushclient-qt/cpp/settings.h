#pragma once
#include <QtCore/QSettings>

typedef struct RecentFileResult
{
  bool changed;
  QStringList recentFiles;
} RecentFileResult;

class Settings
{
public:
  Settings();

  QStringList recentFiles() const;
  RecentFileResult addRecentFile(const QString &path);
  RecentFileResult removeRecentFile(const QString &path);

private:
  QSettings store;
};
