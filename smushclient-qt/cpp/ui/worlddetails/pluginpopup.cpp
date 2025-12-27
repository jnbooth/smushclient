#include "pluginpopup.h"
#include "smushclient_qt/src/ffi/plugin_details.cxxqt.h"
#include "ui_pluginpopup.h"

PluginPopup::PluginPopup(const PluginDetails& plugin, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::PluginPopup)
{
  ui->setupUi(this);
  ui->author->setText(plugin.getAuthor());
  ui->description->setText(plugin.getDescription());
  ui->file->setText(plugin.getFile());
  ui->id->setText(plugin.getId());
  ui->name->setText(plugin.getName());
  ui->updated->setText(plugin.getModified().toString());
  ui->version->setText(plugin.getVersion());
  ui->written->setText(plugin.getWritten().toString());
}

PluginPopup::~PluginPopup()
{
  delete ui;
}
