#ifndef APP_H
#define APP_H

#include <QtGui/QAction>
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

private:
  Ui::App *ui;
  QList<QAction *> recentFileActions;
  QString saveFilter;

  void addRecentFile(const QString &filePath);
  void openRecentFile(qsizetype index);
  void openWorld(const QString &filePath);
  void setupRecentFiles(const QStringList &recentFiles);
  void setWorldMenusEnabled(bool enabled);

private slots:
  void on_action_new_triggered();
  void on_action_open_world_triggered();
  void on_action_save_world_details_as_triggered();
  void on_action_save_world_details_triggered();
  void on_action_world_properties_triggered();
  void on_world_tabs_currentChanged(int index);

  void on_action_rec_1_triggered() { openRecentFile(0); }
  void on_action_rec_2_triggered() { openRecentFile(1); }
  void on_action_rec_3_triggered() { openRecentFile(2); }
  void on_action_rec_4_triggered() { openRecentFile(3); }
  void on_action_rec_5_triggered() { openRecentFile(4); }
};
#endif // APP_H
