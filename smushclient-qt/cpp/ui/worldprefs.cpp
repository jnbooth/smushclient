#include "worldprefs.h"
#include "ui_worldprefs.h"
#include "prefs/address.h"
#include "prefs/aliases.h"
#include "prefs/connecting.h"
#include "prefs/logging.h"
#include "prefs/mud.h"
#include "prefs/numpad.h"
#include "prefs/output.h"
#include "prefs/plugins.h"
#include "prefs/scripts.h"
#include "prefs/timers.h"
#include "prefs/triggers.h"
#include "../scripting/scriptapi.h"

// Public methods

WorldPrefs::WorldPrefs(World &world, SmushClient &client, ScriptApi *api, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::WorldPrefs),
      api(api),
      client(client),
      pane(nullptr),
      world(world)
{
  ui->setupUi(this);
  ui->settings_list->setCurrentRow(0);
}

WorldPrefs::~WorldPrefs()
{
  delete ui;
}

// Private methods

QWidget *WorldPrefs::paneForIndex(int n)
{
  switch (n)
  {
  case 0:
    return new PrefsAddress(world, this);
  case 1:
    return new PrefsConnecting(world, this);
  case 2:
    return new PrefsOutput(world, this);
  case 3:
    return new PrefsMud(world, this);
  case 4:
    return new PrefsLogging(world, this);
  case 5:
    return new PrefsNumpad(world, this);
  case 6:
    return new PrefsAliases(world, client, this);
  case 7:
    return new PrefsTimers(world, client, api->timekeeper, this);
  case 8:
    return new PrefsTriggers(world, client, this);
  case 9:
    return new PrefsScripts(world, this);
  case 10:
    return new PrefsPlugins(client, api, this);
  default:
    return nullptr;
  }
}

// Private slots

void WorldPrefs::on_settings_list_currentRowChanged(int row)
{
  if (row == -1)
    return;

  if (pane)
    delete pane;

  pane = paneForIndex(row);
  ui->contents->addWidget(pane);
}
