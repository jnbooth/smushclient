#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>

class FindDialog;

namespace Ui
{
  class Notepad;
}

class Notepad : public QMainWindow
{
  Q_OBJECT

public:
  explicit Notepad(QWidget *parent = nullptr);
  ~Notepad();

  QTextEdit *editor() const;

private:
  Ui::Notepad *ui;
  FindDialog *findDialog;
  qreal fontSize;

  void applyFontSize();

private slots:
  void on_action_close_all_notepad_windows_triggered();
  void on_action_decrease_size_triggered();
  void on_action_find_triggered();
  void on_action_find_again_triggered();
  void on_action_global_preferences_triggered();
  void on_action_increase_size_triggered();
  void on_action_print_triggered();
  void on_action_reset_size_triggered();
  void on_action_visit_api_guide_triggered();
  void on_action_visit_bug_reports_triggered();
};

class Notepads : public QWidget
{
  Q_OBJECT

public:
  explicit Notepads(QWidget *parent = nullptr);

  void closeAll();
  QTextEdit *pad(const QString &name = QString());

public slots:

private:
  Notepad *create(const QString &name);
};
