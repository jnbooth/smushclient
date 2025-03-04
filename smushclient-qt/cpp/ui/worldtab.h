#pragma once
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QRegularExpression>
#include <QtCore/QPointer>
#include <QtGui/QFontDatabase>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtNetwork/QSslSocket>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QSplitter>
#include "../client.h"
#include "../bridge/document.h"
#include "../scripting/callbacktrigger.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"

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
  void setStatusBarVisible(bool visible);
  void start();
  void stopSound() const;
  const QString &title() const noexcept;
  bool updateWorld();
  constexpr const QString &worldFilePath() const noexcept
  {
    return filePath;
  }

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

public:
  Ui::WorldTab *ui;
  SmushClient client;
  QSslSocket *socket;
  World world;

protected:
  void closeEvent(QCloseEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  void applyWorld();
  void finishDrag();
  bool restoreHistory();
  bool saveHistory() const;
  bool saveWorldAndState(const QString &filePath);
  bool sendCommand(const QString &command, CommandSource source);
  void setupWorldScriptWatcher();
  void updateWorldScript();

private slots:
  void confirmReloadWorldScript(const QString &worldScriptPath);
  void finishResize();
  void flushOutput();
  bool loadPlugins();
  void onAutoScroll(int min, int max);
  void onNewActivity();
  void readFromSocket();
  void onSocketConnect();
  void onSocketDisconnect();
  void onSocketError(QAbstractSocket::SocketError socketError);

  void on_input_copyAvailable(bool available);
  void on_input_submitted(const QString &text);
  void on_input_textChanged();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_copyAvailable(bool available);
  void on_output_customContextMenuRequested(const QPoint &pos);

private:
  bool alertNewActivity = false;
  ScriptApi *api;
  QMetaObject::Connection autoScroll;
  QFont defaultFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  Document *document;
  QString filePath{};
  QTimer *flushTimer;
  bool handleKeypad = false;
  bool initialized = false;
  bool inputCopyAvailable = false;
  bool isActive = true;
  bool manualDisconnect = false;
  std::optional<CallbackTrigger> onDragMove = std::nullopt;
  QPointer<Hotspot> onDragRelease = nullptr;
  bool outputCopyAvailable = false;
  bool queuedConnect = false;
  QTimer *resizeTimer;
  int sessionStartBlock = 0;
  QRegularExpression splitter{};
  bool tryingSsl = false;
  bool useSplitter = false;
  QFileSystemWatcher worldScriptWatcher;
};
