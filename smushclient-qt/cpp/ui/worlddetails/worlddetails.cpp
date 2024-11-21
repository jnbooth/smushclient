#include "worlddetails.h"
#include "ui_worlddetails.h"
#include "aliases.h"
#include "connecting.h"
#include "login.h"
#include "logging.h"
#include "mud.h"
#include "numpad.h"
#include "output.h"
#include "plugins.h"
#include "scripting.h"
#include "timers.h"
#include "triggers.h"
#include "../../scripting/scriptapi.h"
#include "../../model/alias.h"
#include "../../model/plugin.h"
#include "../../model/timer.h"
#include "../../model/trigger.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

// Public methods

WorldPrefs::WorldPrefs(World &world, SmushClient &client, ScriptApi *api, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::WorldPrefs),
      api(api),
      client(client),
      dirty(false),
      pane(nullptr),
      world(world)
{
  ui->setupUi(this);
  ui->settings_list->setCurrentRow(0);
  aliases = new AliasModel(client, this);
  plugins = new PluginModel(client, this);
  timers = new TimerModel(client, api->timekeeper, this);
  triggers = new TriggerModel(client, this);
  connectModel(aliases);
  connectModel(plugins);
  connectModel(timers);
  connectModel(triggers);
}

WorldPrefs::~WorldPrefs()
{
  delete ui;
}

// Private methods

void WorldPrefs::connectModel(QAbstractItemModel *model)
{
  connect(model, &QAbstractItemModel::dataChanged, this, &WorldPrefs::markDirty);
  connect(model, &QAbstractItemModel::layoutChanged, this, &WorldPrefs::markDirty);
  connect(model, &QAbstractItemModel::modelReset, this, &WorldPrefs::markDirty);
}

QWidget *WorldPrefs::paneForIndex(int n)
{
  switch (n)
  {
  case 0:
    return new PrefsConnecting(world, this);
  case 1:
    return new PrefsLogin(world, this);
  case 2:
    return new PrefsOutput(world, this);
  case 3:
    return new PrefsMud(world, this);
  case 4:
    return new PrefsLogging(world, this);
  case 5:
    return new PrefsNumpad(world, this);
  case 6:
    return new PrefsAliases(world, aliases, this);
  case 7:
    return new PrefsTimers(world, timers, this);
  case 8:
    return new PrefsTriggers(world, triggers, this);
  case 9:
    return new PrefsScripting(world, this);
  case 10:
    return new PrefsPlugins(plugins, api, this);
  default:
    return nullptr;
  }
}

// Private slots

void WorldPrefs::markDirty()
{
  dirty = true;
}

void WorldPrefs::on_settings_list_currentRowChanged(int row)
{
  if (row == -1)
    return;

  if (pane)
    delete pane;

  pane = paneForIndex(row);
  ui->contents->addWidget(pane);
}
