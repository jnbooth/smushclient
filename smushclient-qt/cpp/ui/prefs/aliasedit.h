#pragma once
#include "cxx-qt-gen/ffi.cxxqt.h"
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

namespace Ui
{
  class AliasEdit;
}

class AliasEdit : public QDialog
{
  Q_OBJECT

public:
  explicit AliasEdit(Alias *alias, QWidget *parent = nullptr);
  ~AliasEdit();

private slots:
  void on_Text_textChanged();

private:
  Ui::AliasEdit *ui;
  Alias *alias;
};
