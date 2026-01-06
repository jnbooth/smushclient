#ifndef REGEXDIALOG_H
#define REGEXDIALOG_H

#include <QDialog>

namespace Ui {
class RegexDialog;
}

class RegexDialog : public QDialog
{
  Q_OBJECT

public:
  explicit RegexDialog(const QString& message,
                       int offset,
                       const QString& pattern,
                       QWidget* parent = nullptr);
  ~RegexDialog();

private:
  Ui::RegexDialog* ui;
};

#endif // REGEXDIALOG_H
