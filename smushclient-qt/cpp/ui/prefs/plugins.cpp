#include "plugins.h"
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include "ui_plugins.h"
#include "../../environment.h"
#include "../../scripting/scriptapi.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

// Public methods

PrefsPlugins::PrefsPlugins(SmushClient &client, ScriptApi *api, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsPlugins),
      api(api),
      client(client)
{
  ui->setupUi(this);
  buildTable();
}

PrefsPlugins::~PrefsPlugins()
{
  delete ui;
}

// Private methods

void PrefsPlugins::buildTable()
{
  ui->table->clear();
  TableBuilder builder(ui->table);
  client.buildPluginsTable(builder);
}

void PrefsPlugins::initPlugins()
{
  api->initializePlugins(client.pluginScripts());
}

// Private slots

void PrefsPlugins::on_button_add_clicked()
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
  initPlugins();
  buildTable();
}

void PrefsPlugins::on_button_reinstall_clicked()
{
  initPlugins();
}

void PrefsPlugins::on_button_remove_clicked()
{
  const QTableWidgetItem *item = ui->table->currentItem();
  if (!item)
    return;
  if (!client.removePlugin(item->data(Qt::UserRole).toString()))
    return;

  buildTable();
}
