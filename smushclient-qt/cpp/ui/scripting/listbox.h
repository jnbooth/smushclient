#pragma once
#include "abstractscriptdialog.h"
#include <QtWidgets/QListWidgetItem>

namespace Ui {
class ListBoxDialog;
} // namespace Ui

class ListBoxDialog : public AbstractScriptDialog
{
  Q_OBJECT

public:
  explicit ListBoxDialog(const QString& title,
                         const QString& message,
                         QWidget* parent);
  ~ListBoxDialog() override;

  void addItems(const QStringList& items);
  void setMode(QListWidget::SelectionMode mode);
  QString text() const;

  void addItem(const QString& text,
               const QVariant& value,
               bool active) override;
  void sortItems() override;
  QVariant value() const override;

private slots:
  void on_items_itemDoubleClicked(QListWidgetItem* item);

private:
  Ui::ListBoxDialog* ui;
};
