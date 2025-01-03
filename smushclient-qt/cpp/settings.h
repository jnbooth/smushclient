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
  enum class StartupBehavior
  {
    None,
    Reopen,
    List,
  };

  Q_ENUM(StartupBehavior)

  enum class WorldCloseBehavior
  {
    Save,
    Confirm,
    Discard,
  };

  Q_ENUM(WorldCloseBehavior)

public:
  explicit Settings(QObject *parent = nullptr);

  bool getAutoConnect() const;

  QString getBellSound() const;

  bool getConfirmQuit() const;

  bool getDisplayConnect() const;
  bool getDisplayDisconnect() const;

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

  QStringList getOpenAtStartup() const;

  QTextBlockFormat getOutputBlockFormat() const;
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

  StartupBehavior getStartupBehavior() const;
  QString getStartupDirectory() const;
  QString getStartupDirectoryOrDefault() const;
  QStringList getStartupWorlds() const;

  WorldCloseBehavior getWorldCloseBehavior() const;

public slots:
  void setAutoConnect(bool enabled);

  void setBellSound(const QString &sound);

  void setConfirmQuit(bool confirm);

  void setDisplayConnect(bool display);
  void setDisplayDisconnect(bool display);

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

  void setOpenAtStartup(const QStringList &list);

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

  void setStartupBehavior(StartupBehavior behavior);
  void setStartupDirectory(const QString &dir);

  void setWorldCloseBehavior(WorldCloseBehavior behavior);

private:
  QSettings store{};
};
