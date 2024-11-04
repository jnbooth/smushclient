#include "worldprefs.h"
#include "ui_worldprefs.h"
#include "prefs/address.h"
#include "prefs/aliases.h"
#include "prefs/connecting.h"
#include "prefs/logging.h"
#include "prefs/mud.h"
#include "prefs/numpad.h"
#include "prefs/output.h"
#include "prefs/scripts.h"
#include "prefs/timers.h"
#include "prefs/triggers.h"

// Public methods

WorldPrefs::WorldPrefs(World &world, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::WorldPrefs),
      panes(),
      activePane(0)
{
  ui->setupUi(this);
  panes.reserve(10);

  setupPane(new PrefsAddress(world, this), "IP address");
  setupPane(new PrefsConnecting(world, this), "Connecting");
  setupPane(new PrefsLogging(world, this), "Logging");
  setupPane(new PrefsTimers(world, this), "Timers");
  setupPane(new PrefsOutput(world, this), "Output");
  setupPane(new PrefsMud(world, this), "MUD");
  setupPane(new PrefsAliases(world, this), "Aliases");
  setupPane(new PrefsTriggers(world, this), "Triggers");
  setupPane(new PrefsNumpad(world, this), "Keypad");
  setupPane(new PrefsScripts(world, this), "Scripts");
}

WorldPrefs::~WorldPrefs()
{
  delete ui;
}

// Private methods

void WorldPrefs::setupPane(QWidget *pane, const char *key)
{
  const int index = panes.size();
  panes.append(pane);
  ui->contents->addWidget(pane);
  pane->hide();

  QListWidgetItem *item =
      ui->settings_list
          ->findItems(tr(key), Qt::MatchExactly)
          .constFirst();
  item->setData(Qt::UserRole, index);

  if (index == 0)
    ui->settings_list->setCurrentItem(item);
}

// Private slots

void WorldPrefs::on_settings_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
  if (!current)
    return;

  const QVariant data = current->data(Qt::UserRole);
  if (!data.canConvert<qsizetype>())
    return;

  if (previous)
    panes.at(activePane)->hide();

  activePane = data.value<qsizetype>();
  panes.at(activePane)->show();
}
