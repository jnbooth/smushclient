#include "plugins.h"
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include "ui_plugins.h"
#include "../../environment.h"
#include "../../scripting/scriptapi.h"
#include "../../settings.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

// Public methods

PrefsPlugins::PrefsPlugins(SmushClient &client, ScriptApi *api, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsPlugins),
      api(api),
      client(client)
{
  ui->setupUi(this);
  builder = new ModelBuilder(this);
  builder->setHeaders({tr("Name"),
                       tr("Purpose"),
                       tr("Author"),
                       tr("Path"),
                       tr("Enabled"),
                       tr("Version")});
  ui->table->setModel(model());
  buildTable();
}

PrefsPlugins::~PrefsPlugins()
{
  Settings().setHeaderState(ModelType::Plugin, ui->table->horizontalHeader()->saveState());
  delete ui;
}

// Private methods

QString PrefsPlugins::activePluginId()
{
  return model()->data(ui->table->currentIndex(), Qt::UserRole + 1).toString();
}

void PrefsPlugins::buildTable()
{
  QHeaderView *header = ui->table->horizontalHeader();
  const QByteArray headerState =
      model()->rowCount() == 0 ? Settings().headerState(ModelType::Plugin) : header->saveState();

  builder->clear();
  client.buildPluginsTable(*builder);

  header->restoreState(headerState);
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
  const rust::Vec<PluginPack> packs = client.reinstallPlugin(activePluginId());
  if (packs.size() == 1)
    api->reinstallPlugin(packs.front());
  else
    api->initializePlugins(packs);
}

void PrefsPlugins::on_button_remove_clicked()
{
  if (!client.removePlugin(activePluginId()))
    return;

  buildTable();
}
