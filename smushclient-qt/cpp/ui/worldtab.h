#pragma once
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QPointer>
#include <QtCore/QTimerEvent>
#include <QtGui/QResizeEvent>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
#include "../scripting/callbacktrigger.h"
#include "../settings.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

class Document;
class Hotspot;
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
  explicit WorldTab(QWidget *parent = nullptr);
  ~WorldTab();

  AvailableCopy availableCopy() const;
  void closeLog();
  void connectToHost();
  QTextEdit *copyableEditor() const;
  void createWorld() &;
  void disconnectFromHost();
  void editWorldScript();
  void onTabSwitch(bool active) const;
  void openLog();
  void openPluginsDialog();
  bool openWorld(const QString &filename) &;
  bool openWorldSettings();
  void reloadWorldScript() const;
  QString saveWorld(const QString &saveFilter);
  QString saveWorldAsNew(const QString &saveFilter);
  void setOnDragMove(CallbackTrigger &&trigger);
  void setOnDragRelease(Hotspot *hotspot);
  void start();
  const QString title() const noexcept;
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
  void onOutputFontChanged(const QFont &font);

signals:
  void copyAvailable(AvailableCopy available);

protected:
  void leaveEvent(QEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

private:
  ScriptApi *api;
  QFont defaultFont;
  Document *document;
  QString filePath;
  int flushTimerId;
  bool handleKeypad;
  bool initialized;
  bool inputCopyAvailable;
  bool manualDisconnect;
  std::optional<CallbackTrigger> onDragMove;
  QPointer<Hotspot> onDragRelease;
  bool outputCopyAvailable;
  bool queuedConnect;
  int resizeTimerId;
  QRegularExpression splitter;
  bool useSplitter;
  QFileSystemWatcher worldScriptWatcher;

  void applyWorld();
  void finishDrag();
  bool saveWorldAndState(const QString &filePath) const;
  bool sendCommand(const QString &command, CommandSource source);
  void setupWorldScriptWatcher();
  void updateWorldScript();

private slots:
  void confirmReloadWorldScript(const QString &worldScriptPath);
  bool loadPlugins();
  void onConnect();
  void onDisconnect();
  void readFromSocket();

  void on_input_copyAvailable(bool available);
  void on_input_submitted(const QString &text);
  void on_input_textChanged();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_copyAvailable(bool available);
  void on_output_customContextMenuRequested(const QPoint &pos);
};
