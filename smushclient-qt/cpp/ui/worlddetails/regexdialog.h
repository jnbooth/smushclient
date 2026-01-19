#pragma once

#include <QDialog>

namespace Ui {
class RegexDialog;
}

struct RegexParse;

class RegexDialog : public QDialog
{
  Q_OBJECT

public:
  explicit RegexDialog(const RegexParse& parse, QWidget* parent = nullptr);
  ~RegexDialog();

private:
  Ui::RegexDialog* ui;
};
