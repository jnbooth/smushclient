#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidgetItem>

namespace Ui
{
  class ListBox;
}

class ListBox : public QDialog
{
  Q_OBJECT

public:
  explicit ListBox(const QString &title, const QString &message, QWidget *parent = nullptr);
  ~ListBox();

  void addItem(const QString &text, const QVariant &value, bool active = false);
  void sortItems() const;
  QVariant value() const;

private slots:
  void on_items_itemDoubleClicked(QListWidgetItem *item);

private:
  Ui::ListBox *ui;
};
