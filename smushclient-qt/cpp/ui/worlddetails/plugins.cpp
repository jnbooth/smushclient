#include "plugins.h"
#include <QtWidgets/QErrorMessage>
#include <QtWidgets/QFileDialog>
#include "ui_plugins.h"
#include "pluginpopup.h"
#include "../../environment.h"
#include "../../localization.h"
#include "../../model/plugin.h"
#include "../../scripting/scriptapi.h"
#include "../../settings.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

// Public methods

PrefsPlugins::PrefsPlugins(PluginModel *model, ScriptApi *api, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsPlugins),
      api(api),
      model(model)
{
  ui->setupUi(this);
  ui->table->setModel(model);
  ui->table->horizontalHeader()->restoreState(Settings().getHeaderState(objectName()));
  connect(model, &PluginModel::clientError, this, &PrefsPlugins::onClientError);
  connect(model, &PluginModel::pluginOrderChanged, this, &PrefsPlugins::onPluginOrderChanged);
  connect(model, &PluginModel::pluginScriptChanged, this, &PrefsPlugins::onPluginScriptChanged);
}

PrefsPlugins::~PrefsPlugins()
{
  Settings().setHeaderState(objectName(), ui->table->horizontalHeader()->saveState());
  delete ui;
}

// Private slots

void PrefsPlugins::onClientError(const QString &error)
{
  QErrorMessage::qtHandler()->showMessage(error);
}

void PrefsPlugins::onPluginOrderChanged()
{
  api->initializePlugins();
}

void PrefsPlugins::onPluginScriptChanged(size_t index)
{
  api->reinstallPlugin(index);
}

void PrefsPlugins::on_button_add_clicked()
{
  const QString filePath = QFileDialog::getOpenFileName(
      this,
      tr("Add Plugin"),
      QStringLiteral(PLUGINS_DIR),
      FileFilter::plugin());

  if (filePath.isEmpty())
    return;

  model->addPlugin(filePath);
}

void PrefsPlugins::on_button_reinstall_clicked()
{
  model->reinstall(ui->table->currentIndex());
}

void PrefsPlugins::on_button_remove_clicked()
{
  model->removeRow(ui->table->currentIndex().row());
}

void PrefsPlugins::on_button_showinfo_clicked()
{
  PluginPopup(model->pluginDetails(ui->table->currentIndex()), this).exec();
}

void PrefsPlugins::on_table_clicked()
{
  ui->button_reinstall->setEnabled(true);
  ui->button_remove->setEnabled(true);
  ui->button_showinfo->setEnabled(true);
}
