#ifndef WORLDTAB_H
#define WORLDTAB_H

#include <QtNetwork/QTcpSocket>
#include <QtWidgets/QSplitter>
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
  bool openWorld(const QString &filename);
  const QString &title() const;

private:
  Ui::WorldTab *ui;
  QTcpSocket socket;
  Document document;
  SmushClient client;
  World world;

private slots:
  void on_close_worldprefs(int result);
  void on_socket_ready_read();
};

#endif // WORLDTAB_H
