#include "listbox.h"
#include "ui_listbox.h"

// Public methods

ListBoxDialog::ListBoxDialog(const QString& title,
                             const QString& message,
                             QWidget* parent)
  : AbstractScriptDialog(parent)
  , ui(new Ui::ListBoxDialog)
{
  ui->setupUi(this);
  setWindowTitle(title);
  ui->label->setText(message);
}

ListBoxDialog::~ListBoxDialog()
{
  delete ui;
}

void
ListBoxDialog::addItems(const QStringList& items)
{
  ui->items->addItems(items);
}

void
ListBoxDialog::setMode(QListWidget::SelectionMode mode)
{
  ui->items->setSelectionMode(mode);
}

QString
ListBoxDialog::text() const
{
  QListWidgetItem* item = ui->items->currentItem();
  return (item != nullptr) ? item->text() : QString();
}

// Public overrides

void
ListBoxDialog::addItem(const QString& text, const QVariant& value, bool active)
{
  QListWidgetItem* item = new QListWidgetItem(text, ui->items);
  item->setData(Qt::UserRole, value);
  if (active) {
    item->setSelected(true);
  }
}

void
ListBoxDialog::sortItems()
{
  ui->items->sortItems(Qt::SortOrder::AscendingOrder);
}

QVariant
ListBoxDialog::value() const
{
  if (ui->items->selectionMode() ==
      QListWidget::SelectionMode::SingleSelection) {
    QListWidgetItem* item = ui->items->currentItem();
    return (item != nullptr) ? item->data(Qt::UserRole) : QVariant();
  }
  QList<QListWidgetItem*> choices = ui->items->selectedItems();
  QList<QVariant> values;
  values.reserve(choices.size());
  for (const QListWidgetItem* item : choices) {
    values.push_back(item->data(Qt::UserRole));
  }
  return values;
}

void
ListBoxDialog::on_items_itemDoubleClicked(QListWidgetItem* /*item*/)
{
  accept();
}
