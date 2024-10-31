#include "listbox.h"
#include "ui_listbox.h"

ListBox::ListBox(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ListBox)
{
  setWindowTitle(title);
  ui->setupUi(this);
  ui->label->setText(message);
}

ListBox::~ListBox()
{
  delete ui;
}

void ListBox::addItem(const QString &text, const QVariant &value, bool active)
{
  QListWidgetItem *item = new QListWidgetItem(text, ui->items);
  item->setData(Qt::UserRole, value);
  if (active)
    item->setSelected(true);
}

void ListBox::sortItems() const
{
  ui->items->sortItems(Qt::SortOrder::AscendingOrder);
}

QVariant ListBox::value() const
{
  QListWidgetItem *item = ui->items->currentItem();
  return item ? item->data(Qt::UserRole) : QVariant();
}

void ListBox::on_items_itemDoubleClicked(QListWidgetItem *)
{
  accept();
}
