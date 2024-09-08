#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

namespace Ui
{
  class PrefsProxy;
}

class PrefsProxy : public QDialog
{
  Q_OBJECT

public:
  explicit PrefsProxy(QWidget *parent = nullptr);
  ~PrefsProxy();

private:
  Ui::PrefsProxy *ui;
};
