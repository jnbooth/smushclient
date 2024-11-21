#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class PrefsLogin;
}

class World;

class PrefsLogin : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsLogin(World &world, QWidget *parent = nullptr);
  ~PrefsLogin();

private slots:
  void on_ConnectText_textChanged();
  void on_ConnectText_blockCountChanged(int count);

private:
  Ui::PrefsLogin *ui;
  bool empty;
  bool single;
  World &world;
};
