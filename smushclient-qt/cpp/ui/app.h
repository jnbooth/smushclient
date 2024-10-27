#pragma once
#include <QtGui/QAction>
#include <QtWidgets/QMainWindow>
#include "finddialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
  class App;
}
QT_END_NAMESPACE

class WorldTab;

class App : public QMainWindow
{
  Q_OBJECT

public:
  explicit App(QWidget *parent = nullptr);
  ~App();

private:
  Ui::App *ui;
  FindDialog findDialog;
  int lastTabIndex;
  QList<QAction *> recentFileActions;
  QString saveFilter;

  void addRecentFile(const QString &filePath) const;
  void openRecentFile(qsizetype index) const;
  void openWorld(const QString &filePath) const;
  void setupRecentFiles(const QStringList &recentFiles) const;
  void setWorldMenusEnabled(bool enabled) const;
  WorldTab *worldtab() const;
  WorldTab *worldtab(int index) const;

private slots:
  void on_action_connect_triggered();
  void on_action_disconnect_triggered();
  void on_action_edit_world_details_triggered();
  void on_action_find_triggered();
  void on_action_find_again_triggered();
  void on_action_global_preferences_triggered();
  void on_action_new_triggered();
  void on_action_open_world_triggered();
  void on_action_plugins_triggered();
  void on_action_save_world_details_as_triggered();
  void on_action_save_world_details_triggered();
  void on_world_tabs_currentChanged(int index);

  void on_action_rec_1_triggered() { openRecentFile(0); }
  void on_action_rec_2_triggered() { openRecentFile(1); }
  void on_action_rec_3_triggered() { openRecentFile(2); }
  void on_action_rec_4_triggered() { openRecentFile(3); }
  void on_action_rec_5_triggered() { openRecentFile(4); }
};
