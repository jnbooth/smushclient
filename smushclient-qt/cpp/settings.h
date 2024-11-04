#pragma once
#include <QtCore/QSettings>
#include <QtGui/QFont>

typedef struct RecentFileResult
{
  const bool changed;
  const QStringList recentFiles;
} RecentFileResult;

class Settings
{
public:
public:
  Settings();

  QFont inputFont() const;
  void setInputFont(const QFont &font);

  QColor inputBackground() const;
  void setInputBackground(const QColor &color);

  QColor inputForeground() const;
  void setInputForeground(const QColor &color);

  QStringList lastFiles() const;
  void setLastFiles(const QStringList &files);

  QFont outputFont() const;
  void setOutputFont(const QFont &font);

  QStringList recentFiles() const;
  RecentFileResult addRecentFile(const QString &path);
  RecentFileResult removeRecentFile(const QString &path);

private:
  QSettings store;
};
