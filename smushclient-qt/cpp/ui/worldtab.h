#ifndef WORLDTAB_H
#define WORLDTAB_H

#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
#include "../document.h"
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

  void openPreferences();
  void createWorld();
  bool openWorld(const QString &filename);
  QString saveWorld(const QString &saveFilter);
  QString saveWorldAsNew(const QString &saveFilter);
  const QString &title() const;

private:
  Ui::WorldTab *ui;
  QFont defaultFont;
  QString filePath;
  QTcpSocket socket;
  Document document;
  SmushClient client;
  World world;

  void applyWorld();
  void connectToHost();
  void sendCommand(const QString &command);

private slots:
  void on_finished(int result);
  void on_input_returnPressed();
  void on_readyRead();
  void on_output_anchorClicked(const QUrl &url);
  void on_output_customContextMenuRequested(const QPoint &pos);
};

#endif // WORLDTAB_H
