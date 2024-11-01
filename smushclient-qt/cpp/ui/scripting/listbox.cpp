#include "listbox.h"
#include "ui_listbox.h"

// Public methods

ListBox::ListBox(const QString &title, const QString &message, QWidget *parent)
    : AbstractScriptDialog(parent),
      ui(new Ui::ListBox)
{
  ui->setupUi(this);
  setWindowTitle(title);
  ui->label->setText(message);
}
ListBox::~ListBox()
{
  delete ui;
}

void ListBox::setMode(QListWidget::SelectionMode mode)
{
  ui->items->setSelectionMode(mode);
}

// Public overrides

void ListBox::addItem(const QString &text, const QVariant &value, bool active)
{
  QListWidgetItem *item = new QListWidgetItem(text, ui->items);
  item->setData(Qt::UserRole, value);
  if (active)
    item->setSelected(true);
}

void ListBox::sortItems()
{
  ui->items->sortItems(Qt::SortOrder::AscendingOrder);
}

QVariant ListBox::value() const
{
  if (ui->items->selectionMode() == QListWidget::SelectionMode::SingleSelection)
  {
    QListWidgetItem *item = ui->items->currentItem();
    return item ? item->data(Qt::UserRole) : QVariant();
  }
  QList<QListWidgetItem *> choices = ui->items->selectedItems();
  QList<QVariant> values;
  values.reserve(choices.size());
  for (const QListWidgetItem *item : choices)
    values.push_back(item->data(Qt::UserRole));
  return values;
}

void ListBox::on_items_itemDoubleClicked(QListWidgetItem *)
{
  accept();
}
