#pragma once
#include "../client.h"
#include "../hotkeys.h"
#include "../scripting/callback/trigger.h"
#include <QtCore/QFileSystemWatcher>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTextEdit>

#ifdef QT_NO_SSL
#include <QtNetwork/QTcpSocket>
#else
#include <QtNetwork/QSslSocket>
#endif

namespace Ui {
class WorldTab;
} // namespace Ui

enum class ScriptRecompile : int32_t;
enum class SendTo : uint8_t;
class Hotspot;
class MudStatusBar;
class Notepads;
class ScriptApi;

class WorldTab : public QSplitter
{
  Q_OBJECT

public:
  enum class AvailableCopy
  {
    None,
    Input,
    Output,
  };

public:
  explicit WorldTab(Notepads& notepads, QWidget* parent = nullptr);
  ~WorldTab() override;

  bool active() const noexcept { return m_active; }
  AvailableCopy availableCopy() const noexcept;
  void clearCallbacks();
  void clearCallbacks(const Plugin& plugin);
  void closeLog();
  void connectToHost();
  QTextEdit* copyableEditor() const;
  void disconnectFromHost();
  void editWorldScript();
  bool hasWorldScript() const { return !worldScriptPath.isEmpty(); }
  bool importWorld(const QString& filename) &;
  bool isConnected() const;
  void openLog();
  bool openWorld(const QString& filename) &;
  bool openWorldSettings();
  bool promptSave();
  void reloadWorldScript() const;
  void resetAllTimers() const;
  QString saveWorld();
  QString saveWorldAsNew();
  const QHash<QString, QString>& serverStatus() const;
  void setActive(bool active);
  void setOnDragMove(const Plugin& plugin,
                     const PluginCallback& callback,
                     QObject* parent);
  void setOnDragRelease(Hotspot* hotspot);
  void setStatusBarVisible(bool visible);
  ApiCode setWorldOption(size_t pluginIndex,
                         std::string_view name,
                         int64_t value);
  ApiCode setWorldAlphaOption(size_t pluginIndex,
                              std::string_view name,
                              std::string_view value);
  void simulateOutput(std::string_view output) const;
  void start();
  MudStatusBar* statusBar() const;
  void stopSound() const;
  constexpr const QString& title() const noexcept { return worldName; };
  constexpr const QString& worldFilePath() const noexcept { return filePath; }

public slots:
  void onInputBackgroundChanged(const QColor& color) const;
  void onInputFontChanged(const QFont& font) const;
  void onInputForegroundChanged(const QColor& color) const;
  void onOutputBlockFormatChanged(const QTextBlockFormat& format) const;
  void onOutputFontChanged(const QFont& font) const;
  void onOutputPaddingChanged(qreal padding) const;

signals:
  void connectionStatusChanged(bool connected);
  void copyAvailable(AvailableCopy available);
  void newActivity(WorldTab* self);
  void titleChanged(WorldTab* self, const QString& title);

public:
  Ui::WorldTab* ui;

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
  void onAutoScroll(int min, int max) const;
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
  void on_output_copyAvailable(bool available);
  void on_output_customContextMenuRequested(const QPoint& pos);
  void on_output_linkActivated(const QString& action, SendTo sendTo);

private:
  bool alertNewActivity = false;
  ScriptApi* api;
  QMetaObject::Connection autoScroll;
  SmushClient client;
  Document* document;
  QString filePath;
  QTimer* flushTimer;
  bool handleKeypad = false;
  Hotkeys hotkeys;
  bool initialized = false;
  bool inputCopyAvailable = false;
  bool manualDisconnect = false;
  bool m_active = true;
  std::optional<CallbackTrigger> onDragMove = std::nullopt;
  QPointer<Hotspot> onDragRelease = nullptr;
  bool outputCopyAvailable = false;
  bool queuedConnect = false;
  QTimer* resizeTimer;
  ScriptRecompile scriptReloadOption;
#ifdef QT_NO_SSL
  QTcpSocket* socket;
#else
  QSslSocket* socket;
#endif
  int sessionStartBlock = 0;
  QChar splitOn{ u'\n' };
  QString worldName;
  QString worldScriptPath;
  QFileSystemWatcher worldScriptWatcher;
};
