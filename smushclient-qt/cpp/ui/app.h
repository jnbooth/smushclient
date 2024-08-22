#ifndef APP_H
#define APP_H

#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui
{
  class App;
}
QT_END_NAMESPACE

class App : public QMainWindow
{
  Q_OBJECT

public:
  App(QWidget *parent = nullptr);
  ~App();

private slots:
  void on_action_new_triggered();
  void on_action_open_world_triggered();
  void on_action_world_properties_triggered();

private:
  Ui::App *ui;
};
#endif // APP_H
