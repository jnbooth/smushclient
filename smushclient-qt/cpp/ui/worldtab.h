#pragma once
#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
#include "cxx-qt-gen/ffi.cxxqt.h"

class Document;
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
  void focusInput() const;
  bool openWorld(const QString &filename) &;
  void openWorldSettings() &;
  bool updateWorld();
  QString saveWorld(const QString &saveFilter);
  QString saveWorldAsNew(const QString &saveFilter);
  const QString title() const noexcept;

public:
  Ui::WorldTab *ui;
  QTcpSocket *socket;
  World world;

private:
  ScriptApi *api;
  SmushClient client;
  QFont defaultFont;
  Document *document;
  QString filePath;

  void applyWorld() const;
  void connectToHost() const;
  void sendCommand(const QString &command) const;

private slots:
  void finalizeWorldSettings(int result);
  void readFromSocket();

  void on_input_returnPressed();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_customContextMenuRequested(const QPoint &pos);
};
