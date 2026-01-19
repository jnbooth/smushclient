#include "aliases.h"
#include "../../fieldconnector.h"
#include "../../model/alias.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_aliases.h"

PrefsAliases::PrefsAliases(const World& world,
                           AliasModel* model,
                           QWidget* parent)
  : AbstractPrefsTree(model, parent)
  , ui(new Ui::PrefsAliases)
{
  ui->setupUi(this);
  setTree(ui->tree);
  CONNECT_WORLD(EnableAliases);
}

PrefsAliases::~PrefsAliases()
{
  delete ui;
}

// Protected overrides

void
PrefsAliases::enableSingleButtons(bool enabled)
{
  ui->edit->setEnabled(enabled);
}

void
PrefsAliases::enableMultiButtons(bool enabled)
{
  ui->export_xml->setEnabled(enabled);
  ui->remove->setEnabled(enabled);
}
