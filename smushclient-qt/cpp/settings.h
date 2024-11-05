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
  Settings();

  QFont inputFont() const;
  void setInputFont(const QFont &font);

  QColor inputBackground() const;
  void setInputBackground(const QColor &color);

  QColor inputForeground() const;
  void setInputForeground(const QColor &color);

  QStringList lastFiles() const;
  void setLastFiles(const QStringList &files);

  bool loggingEnabled() const;
  void setLoggingEnabled(bool enabled);

  bool showStatusBar() const;
  void setShowStatusBar(bool show);

  QFont outputFont() const;
  void setOutputFont(const QFont &font);

  bool outputWrapping() const;
  void setOutputWrapping(bool wrapping);

  QStringList recentFiles() const;
  RecentFileResult addRecentFile(const QString &path);
  RecentFileResult removeRecentFile(const QString &path);

private:
  QSettings store;

private:
  template <typename T>
  inline T value(QAnyStringView key, T defaultValue) const
  {
    const QVariant val = store.value(key);
    return val.canConvert<T>() ? val.value<T>() : defaultValue;
  }
};
