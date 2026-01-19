#pragma once
#include "../bridge/document.h"
#include "../client.h"
#include "../hotkeys.h"
#include "../scripting/callbacktrigger.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QPointer>
#include <QtCore/QRegularExpression>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QFontDatabase>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTextEdit>

#ifdef QT_NO_SSL
#include <QtNetwork/QTcpSocket>
#else
#include <QtNetwork/QSslSocket>
#endif

class Hotspot;
class MudStatusBar;
class Notepads;
class ScriptApi;

namespace Ui {
class WorldTab;
} // namespace Ui

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
  explicit WorldTab(MudStatusBar* statusBar,
                    Notepads* notepads,
                    QWidget* parent = nullptr);
  ~WorldTab();

  AvailableCopy availableCopy() const;
  void closeLog();
  void connectToHost();
  QTextEdit* copyableEditor() const;
  void disconnectFromHost();
  void editWorldScript();
  constexpr bool hasWorldScript() const { return !worldScriptPath.isEmpty(); }
  constexpr bool isActive() const { return active; }
  bool importWorld(const QString& filePath) &;
  bool isConnected() const;
  void openLog();
  bool openWorld(const QString& filePath) &;
  bool openWorldSettings();
  bool promptSave();
  void reloadWorldScript() const;
  void resetAllTimers() const;
  QString saveWorld();
  QString saveWorldAsNew();
  constexpr const QHash<QString, QString>& serverStatus() const
  {
    return document->serverStatus();
  }
  void setIsActive(bool active);
  void setOnDragMove(CallbackTrigger&& trigger);
  void setOnDragRelease(Hotspot* hotspot);
  void setStatusBarVisible(bool visible);
  ApiCode setWorldOption(size_t pluginIndex, std::string_view name, int value);
  ApiCode setWorldAlphaOption(size_t pluginIndex,
                              std::string_view name,
                              std::string_view value);
  void simulateOutput(std::string_view line) const;
  void start();
  void stopSound() const;
  constexpr const QString& title() const noexcept { return worldName; };
  constexpr const QString& worldFilePath() const noexcept { return filePath; }

public slots:
  void onInputBackgroundChanged(const QColor& color);
  void onInputFontChanged(const QFont& font);
  void onInputForegroundChanged(const QColor& color);
  void onOutputBlockFormatChanged(const QTextBlockFormat& format);
  void onOutputFontChanged(const QFont& font);
  void onOutputPaddingChanged(double padding);

signals:
  void connectionStatusChanged(bool connected);
  void copyAvailable(AvailableCopy available);
  void newActivity(WorldTab* self);
  void titleChanged(WorldTab* self, const QString& title);

public:
  Ui::WorldTab* ui;
  SmushClient client;
#ifdef QT_NO_SSL
  QTcpSocket* socket;
#else
  QSslSocket* socket;
#endif

protected:
  void closeEvent(QCloseEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

private:
  void applyWorld(const World& world);
  void finishDrag();
  void handleConnect();
  bool restoreHistory();
  bool saveHistory() const;
  bool saveWorldAndState(const QString& filePath);
  bool sendCommand(const QString& command, CommandSource source);
  void setupWorldScriptWatcher();
  void showAliasMenu();
  void updateWorldScript();

private slots:
  void confirmReloadWorldScript(const QString& worldScriptPath);
  void finishResize();
  void flushOutput();
  void loadPlugins();
  void onAliasMenuRequested(const QString& word);
  void onAutoScroll(int min, int max);
  void onNewActivity();
  void readFromSocket();
  void onSocketConnect();
  void onSocketDisconnect();
#ifndef QT_NO_SSL
  void onSocketEncrypted();
#endif
  void onSocketError(QAbstractSocket::SocketError socketError);

  void on_input_copyAvailable(bool available);
  void on_input_submitted(const QString& text);
  void on_input_textChanged();
  void on_output_anchorClicked(const QUrl& url);
  void on_output_copyAvailable(bool available);
  void on_output_customContextMenuRequested(const QPoint& pos);

private:
  bool active = true;
  bool alertNewActivity = false;
  ScriptApi* api;
  QMetaObject::Connection autoScroll;
  Document* document;
  QString filePath{};
  QTimer* flushTimer;
  bool handleKeypad = false;
  Hotkeys hotkeys{};
  bool initialized = false;
  bool inputCopyAvailable = false;
  bool manualDisconnect = false;
  std::optional<CallbackTrigger> onDragMove = std::nullopt;
  QPointer<Hotspot> onDragRelease = nullptr;
  bool outputCopyAvailable = false;
  bool queuedConnect = false;
  QTimer* resizeTimer;
  ScriptRecompile scriptReloadOption = ScriptRecompile::Never;
  int sessionStartBlock = 0;
  QChar splitOn{ u'\n' };
  QString worldName{};
  QString worldScriptPath{};
  QFileSystemWatcher worldScriptWatcher;
};
