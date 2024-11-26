#pragma once
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QPointer>
#include <QtCore/QTimerEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
#include "../bridge/document.h"
#include "../scripting/callbacktrigger.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

class Hotspot;
class MudStatusBar;
class Notepads;
class ScriptApi;

namespace Ui
{
  class WorldTab;
}

enum class AvailableCopy
{
  None,
  Input,
  Output,
};

class WorldTab : public QSplitter
{
  Q_OBJECT

public:
  Q_ENUM(AvailableCopy)

public:
  explicit WorldTab(MudStatusBar *statusBar, Notepads *notepads, QWidget *parent = nullptr);
  ~WorldTab();

  static QString saveFilter();

  AvailableCopy availableCopy() const;
  void closeLog();
  bool connected() const;
  void connectToHost();
  QTextEdit *copyableEditor() const;
  void createWorld() &;
  void disconnectFromHost();
  void editWorldScript();
  void openLog();
  bool openWorld(const QString &filename) &;
  bool openWorldSettings();
  bool promptSave();
  void reloadWorldScript() const;
  void resetAllTimers() const;
  QString saveWorld();
  QString saveWorldAsNew();
  constexpr const QHash<QString, QString> &serverStatus() const
  {
    return document->serverStatus();
  }
  void setIsActive(bool active);
  void setOnDragMove(CallbackTrigger &&trigger);
  void setOnDragRelease(Hotspot *hotspot);
  void start();
  void stopSound() const;
  const QString &title() const noexcept;
  bool updateWorld();
  constexpr const QString &worldFilePath() const noexcept
  {
    return filePath;
  }

public:
  Ui::WorldTab *ui;
  SmushClient client;
  QTcpSocket *socket;
  World world;

public slots:
  void onInputBackgroundChanged(const QColor &color);
  void onInputFontChanged(const QFont &font);
  void onInputForegroundChanged(const QColor &color);
  void onOutputBlockFormatChanged(const QTextBlockFormat &format);
  void onOutputFontChanged(const QFont &font);
  void onOutputPaddingChanged(double padding);

signals:
  void connectionStatusChanged(bool connected);
  void copyAvailable(AvailableCopy available);
  void newActivity(WorldTab *self);

protected:
  void closeEvent(QCloseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

private:
  bool alertNewActivity;
  ScriptApi *api;
  QMetaObject::Connection autoScroll;
  QFont defaultFont;
  Document *document;
  QString filePath;
  int flushTimerId;
  bool handleKeypad;
  bool initialized;
  bool inputCopyAvailable;
  bool isActive;
  bool manualDisconnect;
  std::optional<CallbackTrigger> onDragMove;
  QPointer<Hotspot> onDragRelease;
  bool outputCopyAvailable;
  bool queuedConnect;
  int resizeTimerId;
  int sessionStartBlock;
  QRegularExpression splitter;
  bool useSplitter;
  QFileSystemWatcher worldScriptWatcher;

  void applyWorld();
  void finishDrag();
  bool restoreHistory();
  bool saveHistory() const;
  bool saveState(const QString &filePath) const;
  bool saveWorldAndState(const QString &filePath);
  bool sendCommand(const QString &command, CommandSource source);
  void setupWorldScriptWatcher();
  void updateWorldScript();

private slots:
  void confirmReloadWorldScript(const QString &worldScriptPath);
  bool loadPlugins();
  void onAutoScroll(int min, int max);
  void onConnect();
  void onDisconnect();
  void onNewActivity();
  void readFromSocket();

  void on_input_copyAvailable(bool available);
  void on_input_submitted(const QString &text);
  void on_input_textChanged();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_copyAvailable(bool available);
  void on_output_customContextMenuRequested(const QPoint &pos);
};
