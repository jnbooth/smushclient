#pragma once
#include "../../enumbounds.h"
#include <QtCore/QUuid>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>

class FindDialog;

namespace Ui {
class Notepad;
} // namespace Ui

class Notepad : public QMainWindow
{
  Q_OBJECT

public:
  enum class SaveMethod : int64_t
  {
    Default,
    AlwaysPrompt,
    NeverPrompt,
  };

public:
  explicit Notepad(QUuid worldID,
                   const QString& name,
                   QWidget* parent = nullptr);
  explicit Notepad(const QString& name, QWidget* parent = nullptr);
  ~Notepad() override;

  QTextDocument* document() const;
  QTextEdit* editor() const;
  constexpr const QUuid& world() const noexcept { return worldID; }

public slots:
  void appendText(const QString& text);
  QString save();
  QString saveAs() { return saveAsNew(QString()); }
  QString saveAsNew(const QString& path, bool separate = false);
  void setSaveMethod(SaveMethod method);
  void setText(const QString& text);

protected:
  void closeEvent(QCloseEvent* event) override;

private:
  bool promptSave();
  QString fileHint() const;
  bool writeToFile(const QString& path);
  void updateWindowTitle(const QString& name);

private slots:
  void on_action_about_triggered();
  void on_action_close_all_notepad_windows_triggered();
  void on_action_decrease_size_triggered();
  void on_action_find_triggered();
  void on_action_find_again_triggered();
  void on_action_font_triggered();
  void on_action_global_preferences_triggered();
  void on_action_increase_size_triggered();
  void on_action_print_triggered();
  void on_action_reset_size_triggered();
  void on_action_save_selection_triggered();
  static void on_action_visit_api_guide_triggered();
  static void on_action_visit_bug_reports_triggered();

private:
  Ui::Notepad* ui;
  qreal defaultFontSize;
  FindDialog* findDialog = nullptr;
  QString filePath;
  bool promptBeforeClose = true;
  QString windowTitleBase;
  QUuid worldID;
};

DECLARE_ENUM_BOUNDS(Notepad::SaveMethod, Default, NeverPrompt)
