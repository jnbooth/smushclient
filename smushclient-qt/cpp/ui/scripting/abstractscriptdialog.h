#pragma once
#include <QtWidgets/QDialog>

class AbstractScriptDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AbstractScriptDialog(QWidget* parent = nullptr)
    : QDialog(parent) {};

  virtual void addItem(const QString& text,
                       const QVariant& value,
                       bool active) = 0;
  virtual void sortItems() = 0;
  virtual QVariant value() const = 0;
};
