#pragma once
#include <QtCore/QSettings>
#include <QtGui/QFont>
#include <QtGui/QTextBlockFormat>

typedef struct RecentFileResult
{
  const bool changed;
  const QStringList recentFiles;
} RecentFileResult;

class Settings : public QObject
{
  Q_OBJECT

public:
  explicit Settings(QObject *parent = nullptr);

  bool getAutoConnect() const;

  QTextBlockFormat getOutputBlockFormat() const;

  QColor getInputBackground() const;
  QFont getInputFont() const;
  QColor getInputForeground() const;
  bool getInputHistoryLimit() const;
  int getInputHistoryLines() const;

  QStringList getLastFiles() const;

  bool getLoggingEnabled() const;

  QFont getNotepadFont() const;
  QColor getNotepadBackground() const;
  QColor getNotepadForeground() const;

  QFont getOutputFont() const;
  bool getOutputHistoryEnabled() const;
  bool getOutputHistoryLimit() const;
  int getOutputHistoryLines() const;
  double getOutputPadding() const;
  bool getOutputLimit() const;
  int getOutputLines() const;
  int getOutputLineSpacing() const;
  bool getOutputWrapping() const;

  QStringList getRecentFiles() const;
  RecentFileResult addRecentFile(const QString &path);
  RecentFileResult removeRecentFile(const QString &path);

  bool getReconnectOnDisconnect() const;

  bool getShowStatusBar() const;

  QByteArray getHeaderState(const QString &modelName) const;

public slots:
  void setAutoConnect(bool enabled);

  void setInputBackground(const QColor &color);
  void setInputFont(const QFont &font);
  void setInputForeground(const QColor &color);
  void setInputHistoryLimit(bool limit);
  void setInputHistoryLines(int lines);

  void setLastFiles(const QStringList &files);

  void setLoggingEnabled(bool enabled);

  void setNotepadFont(const QFont &font);
  void setNotepadBackground(const QColor &color);
  void setNotepadForeground(const QColor &color);

  void setOutputFont(const QFont &font);
  void setOutputHistoryEnabled(bool enabled);
  void setOutputHistoryLimit(bool limit);
  void setOutputHistoryLines(int lines);
  void setOutputPadding(double inset);
  void setOutputLimit(bool limit);
  void setOutputLines(int lines);
  void setOutputLineSpacing(int spacing);
  void setOutputWrapping(bool wrapping);

  void setReconnectOnDisconnect(bool reconnect);

  void setShowStatusBar(bool show);

  void setHeaderState(const QString &modelName, const QByteArray &state);

private:
  QSettings store;
};
