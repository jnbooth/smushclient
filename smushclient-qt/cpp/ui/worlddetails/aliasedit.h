#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class AliasEdit;
}

class AliasEdit : public QDialog
{
  Q_OBJECT

public:
  explicit AliasEdit(Alias &alias, QWidget *parent = nullptr);
  ~AliasEdit();

  bool groupChanged() const;

private slots:
  void on_Label_textChanged(const QString &text);
  void on_UserSendTo_currentIndexChanged(int index);
  void on_Text_textChanged();

private:
  Ui::AliasEdit *ui;
  Alias &alias;
  QString originalGroup;
};
