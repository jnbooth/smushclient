#pragma once
#include <QtGui/QAction>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QMainWindow>
#include "finddialog.h"
#include "../settings.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
  class MainWindow;
}
QT_END_NAMESPACE

enum class AvailableCopy : int;
class Notepads;
class WorldTab;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Notepads *notepads, QWidget *parent = nullptr);
  ~MainWindow();

  void openWorld(const QString &filePath);

protected:
  void closeEvent(QCloseEvent *event) override;
  bool event(QEvent *event) override;

private:
  Ui::MainWindow *ui;
  QIcon activityIcon = QIcon::fromTheme(QIcon::ThemeIcon::DialogWarning);
  FindDialog *findDialog;
  int lastTabIndex = -1;
  Notepads *notepads;
  QList<QAction *> recentFileActions{};
  Settings settings{};
  QMetaObject::Connection socketConnection{};

  static const QString &settingsKey();

  void addRecentFile(const QString &filePath);
  WorldTab *createWorldTab(QWidget *parent) const;
  void connectTab(WorldTab *tab) const;
  void openRecentFile(qsizetype index);
  bool restore();
  void save() const;
  void setupRecentFiles(const QStringList &recentFiles) const;
  void setWorldMenusEnabled(bool enabled) const;
  WorldTab *worldtab() const;
  WorldTab *worldtab(int index) const;

private slots:
  void onCopyAvailable(AvailableCopy copy);
  void onConnectionStatusChanged(bool connected);
  void onNewActivity(WorldTab *tab);

  void on_action_about_triggered();
  void on_action_clear_output_triggered();
  void on_action_close_all_notepad_windows_triggered();
  void on_action_close_world_triggered();
  void on_action_command_history_triggered();
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
  void on_action_new_window_triggered();
  void on_action_open_world_triggered();
  void on_action_paste_triggered();
  void on_action_pause_output_triggered(bool checked);
  void on_action_print_triggered();
  void on_action_quit_triggered();
  void on_action_reload_script_file_triggered();
  void on_action_reset_all_timers_triggered();
  void on_action_save_selection_triggered();
  void on_action_save_world_details_as_triggered();
  void on_action_save_world_details_triggered();
  void on_action_select_all_triggered();
  void on_action_server_status_triggered();
  void on_action_status_bar_triggered(bool checked);
  void on_action_stop_sound_playing_triggered();
  void on_action_wrap_output_triggered(bool checked);
  void on_action_visit_api_guide_triggered();
  void on_action_visit_bug_reports_triggered();
  void on_menu_file_aboutToShow();
  void on_menu_help_aboutToShow();
  void on_menu_view_aboutToShow();
  void on_world_tabs_currentChanged(int index);

  void on_action_rec_1_triggered() { openRecentFile(0); }
  void on_action_rec_2_triggered() { openRecentFile(1); }
  void on_action_rec_3_triggered() { openRecentFile(2); }
  void on_action_rec_4_triggered() { openRecentFile(3); }
  void on_action_rec_5_triggered() { openRecentFile(4); }
  void on_world_tabs_tabCloseRequested(int index);
};
