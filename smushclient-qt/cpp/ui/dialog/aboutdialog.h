#pragma once
#include <QtWidgets/QMessageBox>

class AboutDialog : public QMessageBox
{
  Q_OBJECT

public:
  explicit AboutDialog(QWidget* parent = nullptr);
};
