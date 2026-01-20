#pragma once
#include <QtWidgets/QDialog>

namespace Ui {
class AliasEdit;
} // namespace Ui

class Alias;

class AliasEdit : public QDialog
{
  Q_OBJECT

public:
  explicit AliasEdit(Alias& alias, QWidget* parent = nullptr);
  ~AliasEdit() override;

public slots:
  void accept() override;

private slots:
  void on_Label_textChanged(const QString& text);
  void on_UserSendTo_currentIndexChanged(int index);
  void on_Text_textChanged();

private:
  Ui::AliasEdit* ui;
  Alias& alias;
};
