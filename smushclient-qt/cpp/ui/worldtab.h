#pragma once
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
#include "../bridge/document.h"
#include "../scripting/scriptengine.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

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
  void focusInput() const;
  bool openWorld(const QString &filename) &;
  void openWorldSettings() &;
  QString saveWorld(const QString &saveFilter);
  QString saveWorldAsNew(const QString &saveFilter);
  const QString title() const noexcept;

private:
  Ui::WorldTab *ui;
  SmushClient client;
  QFont defaultFont;
  Document *document;
  QString filePath;
  ScriptEngine scriptEngine;
  QTcpSocket *socket;
  World world;

  void applyWorld();
  void connectToHost();
  void sendCommand(const QString &command);

private slots:
  void finalizeWorldSettings(int result);
  void readFromSocket();

  void on_input_returnPressed();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_customContextMenuRequested(const QPoint &pos);
};
