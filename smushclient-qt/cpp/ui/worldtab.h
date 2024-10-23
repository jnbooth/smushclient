#pragma once
#include <QtCore/QPointer>
#include <QtCore/QTimerEvent>
#include <QtGui/QResizeEvent>
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
#include "../scripting/callbacktrigger.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

class Document;
class Hotspot;
class ScriptApi;

namespace Ui
{
  class WorldTab;
}

class WorldTab : public QSplitter
{
  Q_OBJECT

public:
  explicit WorldTab(QWidget *parent = nullptr);
  ~WorldTab();

  void createWorld() &;
  void onTabSwitch(bool active) const;
  void openPluginsDialog();
  bool openWorld(const QString &filename) &;
  void openWorldSettings() &;
  QString saveWorld(const QString &saveFilter);
  QString saveWorldAsNew(const QString &saveFilter);
  void setOnDragMove(CallbackTrigger &&trigger);
  void setOnDragRelease(Hotspot *hotspot);
  const QString title() const noexcept;
  bool updateWorld();

public:
  Ui::WorldTab *ui;
  SmushClient client;
  QTcpSocket *socket;
  World world;

protected:
  void leaveEvent(QEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

private:
  ScriptApi *api;
  QFont defaultFont;
  Document *document;
  QString filePath;
  std::optional<CallbackTrigger> onDragMove;
  QPointer<Hotspot> onDragRelease;
  int resizeTimerId;

  void applyWorld() const;
  void connectToHost() const;
  void finishDrag();
  void openLog();
  bool saveWorldAndState(const QString &filePath) const;

private slots:
  void finalizeWorldSettings(int result);
  bool loadPlugins();
  void onConnect();
  void onDisconnect();
  void readFromSocket();

  void on_input_returnPressed();
  void on_input_textEdited();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_customContextMenuRequested(const QPoint &pos);
};
