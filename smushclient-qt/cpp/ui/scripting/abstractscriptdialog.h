#pragma once
#include <QtWidgets/QDialog>

class AbstractScriptDialog : public QDialog {
  Q_OBJECT

public:
  explicit AbstractScriptDialog(QWidget *parent = nullptr);
  virtual ~AbstractScriptDialog();

  virtual void addItem(const QString &text, const QVariant &value,
                       bool active = false) = 0;
  virtual void sortItems() = 0;
  virtual QVariant value() const = 0;
};
