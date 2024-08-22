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
  const QString &title() const;

private:
  Ui::WorldTab *ui;
  QTcpSocket socket;
  Document document;
  SmushClient client;
  World world;

  void connectToHost();

private slots:
  void on_finished(int result);
  void on_input_returnPressed();
  void on_readyRead();
};

#endif // WORLDTAB_H
