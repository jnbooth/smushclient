#pragma once
#include <QtCore/QSettings>
#include <QtGui/QFont>

typedef struct RecentFileResult
{
  const bool changed;
  const QStringList recentFiles;
} RecentFileResult;

enum class ModelType
{
  Alias,
  Timer,
  Trigger,
  Plugin,
};

class Settings : public QObject
{
  Q_OBJECT

public:
  explicit Settings(QObject *parent = nullptr);

  bool autoConnect() const;

  QFont inputFont() const;
  QColor inputBackground() const;
  QColor inputForeground() const;

  QStringList lastFiles() const;

  bool loggingEnabled() const;

  QFont notepadFont() const;
  QColor notepadBackground() const;
  QColor notepadForeground() const;

  QFont outputFont() const;
  bool outputWrapping() const;

  QStringList recentFiles() const;
  RecentFileResult addRecentFile(const QString &path);
  RecentFileResult removeRecentFile(const QString &path);

  bool reconnectOnDisconnect() const;

  bool showStatusBar() const;

  QByteArray headerState(ModelType modelType) const;

public slots:
  void setAutoConnect(bool enabled);

  void setInputFont(const QFont &font);
  void setInputBackground(const QColor &color);
  void setInputForeground(const QColor &color);

  void setLastFiles(const QStringList &files);

  void setLoggingEnabled(bool enabled);

  void setNotepadFont(const QFont &font);
  void setNotepadBackground(const QColor &color);
  void setNotepadForeground(const QColor &color);

  void setOutputFont(const QFont &font);
  void setOutputWrapping(bool wrapping);

  void setReconnectOnDisconnect(bool reconnect);

  void setShowStatusBar(bool show);

  void setHeaderState(ModelType modelType, const QByteArray &state);

private:
  QSettings store;

private:
  template <typename T>
  inline T value(QAnyStringView key, T defaultValue) const
  {
    return store.contains(key) ? store.value(key).value<T>() : defaultValue;
  }
};
