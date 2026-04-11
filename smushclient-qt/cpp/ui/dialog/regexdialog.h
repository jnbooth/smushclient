#pragma once

#include <QtWidgets/QDialog>

namespace Ui {
class RegexDialog;
} // namespace Ui

struct ParseResult;

class RegexDialog : public QDialog
{
  Q_OBJECT

public:
  static bool handle(const ParseResult& parse, QWidget* parent = nullptr);

  explicit RegexDialog(const ParseResult& parse, QWidget* parent = nullptr);
  ~RegexDialog() override;

private:
  Ui::RegexDialog* ui;
};
