#include "pluginsdialog.h"
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include "ui_pluginsdialog.h"
#include "../../environment.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

// Public methods

PluginsDialog::PluginsDialog(SmushClient &client, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::PluginsDialog),
      client(client),
      changedPlugins(false)
{
  ui->setupUi(this);
  buildTable();
}

PluginsDialog::~PluginsDialog()
{
  delete ui;
}

// Private methods

void PluginsDialog::buildTable()
{
  ui->table->clear();
  TableBuilder builder(ui->table);
  client.buildPluginsTable(builder);
}

// Private slots

void PluginsDialog::on_button_add_clicked()
{
  const QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Add Plugin"),
      QStringLiteral(PLUGINS_DIR),
      tr("Plugin files (*.xml);;All Files(*.*)"));

  if (filePath.isEmpty())
    return;

  const QString error = client.addPlugin(filePath);
  if (!error.isEmpty())
  {
    QErrorMessage::qtHandler()->showMessage(error);
    return;
  }
  changedPlugins = true;
  buildTable();
}

void PluginsDialog::on_button_close_clicked()
{
  if (changedPlugins)
    accept();
  else
    reject();
}

void PluginsDialog::on_button_reinstall_clicked()
{
  emit reinstallClicked();
}

void PluginsDialog::on_button_remove_clicked()
{
  const QTableWidgetItem *item = ui->table->currentItem();
  if (!item)
    return;
  if (!client.removePlugin(item->data(Qt::UserRole).toString()))
    return;
  changedPlugins = true;
  buildTable();
}
