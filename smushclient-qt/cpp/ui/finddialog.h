#pragma once
#include <QtCore/QRegularExpression>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTextEdit>

namespace Ui {
class FindDialog;
} // namespace Ui

class FindDialog : public QDialog
{
  Q_OBJECT

public:
  explicit FindDialog(QWidget* parent = nullptr);
  ~FindDialog() override;

  void find(QTextEdit* edit) const;
  constexpr bool isFilled() const { return filled; }

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

private:
  Ui::FindDialog* ui;
  bool filled = false;
  QFlags<QTextDocument::FindFlag> flags;
  bool isRegex = false;
  QRegularExpression pattern;
  QString text;
};
