#include "choose.h"
#include "ui_choose.h"

// Public methods

Choose::Choose(const QString &title, const QString &message, QWidget *parent)
    : AbstractScriptDialog(parent), ui(new Ui::Choose) {
  ui->setupUi(this);
  setWindowTitle(title);
  ui->label->setText(message);
}

Choose::~Choose() { delete ui; }

// Public overrides

void Choose::addItem(const QString &text, const QVariant &value, bool active) {
  ui->items->addItem(text, value);
  if (active)
    ui->items->setCurrentIndex(ui->items->count() - 1);
}

void Choose::sortItems() { ui->items->model()->sort(0); }

QVariant Choose::value() const { return ui->items->currentData(); }
