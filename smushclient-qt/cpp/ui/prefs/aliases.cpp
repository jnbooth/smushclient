#include "aliases.h"
#include "ui_aliases.h"

PrefsAliases::PrefsAliases(const World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsAliases)
{
  ui->setupUi(this);
  CONNECT_WORLD(EnableAliases);
}

PrefsAliases::~PrefsAliases()
{
  delete ui;
}
