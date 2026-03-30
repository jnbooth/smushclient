#pragma once
#include <QtGui/QTextCharFormat>
#include <QtWidgets/QDialog>

namespace Ui {
class StyleDialog;
} // namespace Ui

class StyleDialog : public QDialog
{
  Q_OBJECT

public:
  explicit StyleDialog(const QTextCharFormat& format,
                       QWidget* parent = nullptr);
  ~StyleDialog() override;

private:
  Ui::StyleDialog* ui;
};
