#include "listbox.h"
#include "../../scripting/qlua.h"
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

int
ListBoxDialog::pushValue(lua_State* L) const
{
  if (ui->items->selectionMode() ==
      QListWidget::SelectionMode::SingleSelection) {
    QListWidgetItem* item = ui->items->currentItem();
    if (item == nullptr) {
      lua_pushnil(L);
    } else {
      qlua::pushQVariant(L, item->data(Qt::UserRole));
    }
    return 1;
  }
  QList<QListWidgetItem*> choices = ui->items->selectedItems();
  lua_createtable(L, 0, static_cast<int>(choices.size()));
  for (const QListWidgetItem* item : choices) {
    qlua::pushQVariant(L, item->data(Qt::UserRole));
    qlua::push(L, true);
    lua_rawset(L, -3);
  }
  return 1;
}

void
ListBoxDialog::sortItems()
{
  ui->items->sortItems(Qt::SortOrder::AscendingOrder);
}

void
ListBoxDialog::on_items_itemDoubleClicked(QListWidgetItem* /*item*/)
{
  accept();
}
