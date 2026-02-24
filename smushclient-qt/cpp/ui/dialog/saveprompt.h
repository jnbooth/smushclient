#pragma once
#include <QtWidgets/QMessageBox>

class SavePrompt : public QMessageBox
{
  Q_OBJECT

public:
  explicit SavePrompt(const QString& subject, QWidget* parent = nullptr);
};
