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

enum class AvailableCopy : int;
class WorldTab;

class App : public QMainWindow
{
  Q_OBJECT

public:
  explicit App(QWidget *parent = nullptr);
  ~App();

private:
  Ui::App *ui;
  FindDialog *findDialog;
  int lastTabIndex;
  QList<QAction *> recentFileActions;

  void addRecentFile(const QString &filePath) const;
  void openRecentFile(qsizetype index);
  void openWorld(const QString &filePath);
  void setupRecentFiles(const QStringList &recentFiles) const;
  void setWorldMenusEnabled(bool enabled) const;
  WorldTab *worldtab() const;
  WorldTab *worldtab(int index) const;

private slots:
  void onCopyAvailable(AvailableCopy copy);

  void on_action_auto_connect_triggered(bool checked);
  void on_action_clear_output_triggered();
  void on_action_close_world_triggered();
  void on_action_command_history_triggered();
  void on_action_connect_to_all_open_worlds_triggered();
  void on_action_connect_triggered();
  void on_action_copy_triggered();
  void on_action_copy_as_html_triggered();
  void on_action_cut_triggered();
  void on_action_disconnect_triggered();
  void on_action_edit_world_details_triggered();
  void on_action_edit_script_file_triggered();
  void on_action_find_triggered();
  void on_action_find_again_triggered();
  void on_action_global_preferences_triggered();
  void on_action_go_to_line_triggered();
  void on_action_log_session_triggered(bool checked);
  void on_action_new_triggered();
  void on_action_open_world_triggered();
  void on_action_paste_triggered();
  void on_action_pause_output_triggered(bool checked);
  void on_action_plugins_triggered();
  void on_action_quit_triggered();
  void on_action_reconnect_on_disconnect_triggered(bool checked);
  void on_action_reload_script_file_triggered();
  void on_action_save_world_details_as_triggered();
  void on_action_save_world_details_triggered();
  void on_action_select_all_triggered();
  void on_action_status_bar_triggered(bool checked);
  void on_action_wrap_output_triggered(bool checked);
  void on_menu_file_aboutToShow();
  void on_menu_view_aboutToShow();
  void on_world_tabs_currentChanged(int index);

  void on_action_rec_1_triggered() { openRecentFile(0); }
  void on_action_rec_2_triggered() { openRecentFile(1); }
  void on_action_rec_3_triggered() { openRecentFile(2); }
  void on_action_rec_4_triggered() { openRecentFile(3); }
  void on_action_rec_5_triggered() { openRecentFile(4); }
  void on_world_tabs_tabCloseRequested(int index);
};
