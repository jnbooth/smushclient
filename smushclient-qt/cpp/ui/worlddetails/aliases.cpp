#include "aliases.h"
#include "aliasedit.h"
#include "ui_aliases.h"
#include "../../fieldconnector.h"
#include "../../model/alias.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

PrefsAliases::PrefsAliases(const World &world, SmushClient &client, QWidget *parent)
    : AbstractPrefsTree(parent),
      ui(new Ui::PrefsAliases)
{
  ui->setupUi(this);
  model = new AliasModel(client, this);
  setModel(model);
  setTree(ui->tree);
  CONNECT_WORLD(EnableAliases);
  client.stopAliases();
}

PrefsAliases::~PrefsAliases()
{
  delete ui;
}

// Protected overrides

void PrefsAliases::enableSingleButtons(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

void PrefsAliases::enableMultiButtons(bool enabled)
{
  ui->export_xml->setEnabled(enabled);
  ui->remove->setEnabled(enabled);
}
