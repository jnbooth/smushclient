#include "../../model/colormap.h"
#include "colormap.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_colormap.h"
#include <QtWidgets/QColorDialog>

// Public methods

PrefsColorMap::PrefsColorMap(ColorMapModel& model, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::PrefsColorMap)
  , model(model)
{
  ui->setupUi(this);
  ui->table->setModel(&model);
  ui->table->resizeColumnToContents(0);
  ui->table->setColumnWidth(1, ui->table->columnWidth(0));
  ui->table->setColumnWidth(2, 30);
}

PrefsColorMap::~PrefsColorMap()
{
  delete ui;
}

// Private slots

void
PrefsColorMap::on_button_add_clicked()
{
  model.insertRow(model.rowCount());
}

void
PrefsColorMap::on_table_activated(const QModelIndex& index)
{
  const QColor color = QColorDialog::getColor(model.value(index), this);
  if (!color.isValid()) {
    return;
  }
  model.setValue(index, color);
}

void
PrefsColorMap::on_table_clicked(const QModelIndex& index)
{
  if (index.column() == 2) {
    model.removeRow(index.row());
  } else {
    on_table_activated(index);
  }
}
