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
  void sendCommand(const QString &command) const;
  const QString title() const noexcept;

public:
  Ui::WorldTab *ui;
  SmushClient client;
  QTcpSocket *socket;
  World world;

private:
  ScriptApi *api;
  QFont defaultFont;
  Document *document;
  QString filePath;

  void applyWorld() const;
  void connectToHost() const;

private slots:
  void finalizeWorldSettings(int result);
  void readFromSocket();

  void on_input_returnPressed();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_customContextMenuRequested(const QPoint &pos);
};
