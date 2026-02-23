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
  bool filled() const noexcept { return m_filled; }

private slots:
  void on_buttonBox_accepted();
  void on_buttonBox_rejected();

private:
  Ui::FindDialog* ui;
  QTextDocument::FindFlags flags;
  bool isRegex = false;
  bool m_filled = false;
  QRegularExpression pattern;
  QString text;
};
