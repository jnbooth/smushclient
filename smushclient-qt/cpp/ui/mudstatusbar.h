#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class MudStatusBar;
}

class MudStatusBar : public QWidget
{
  Q_OBJECT

public:
  explicit MudStatusBar(QWidget *parent = nullptr);
  ~MudStatusBar();

public slots:
  void setConnected(bool connected);
  void setMessage(const QString &message);
  void setUsers(int users);
  void setUsers(const QString &users);

private:
  Ui::MudStatusBar *ui;
};
