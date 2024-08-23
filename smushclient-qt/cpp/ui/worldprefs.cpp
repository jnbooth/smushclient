#include "worldprefs.h"
#include "ui_worldprefs.h"
#include "prefs/address.h"
#include "prefs/connecting.h"
#include "prefs/logging.h"
#include "prefs/timers.h"
#include "prefs/chat.h"
#include "prefs/output.h"
#include "prefs/mxp.h"
#include "prefs/color.h"
#include "prefs/customcolor.h"
#include "prefs/commands.h"
#include "prefs/aliases.h"
#include "prefs/triggers.h"

WorldPrefs::WorldPrefs(World *world, QWidget *parent)
    : QDialog(parent), ui(new Ui::WorldPrefs)
{
  ui->setupUi(this);
  panes.reserve(12);
  setupPane(new PrefsAddress(world, this), "IP address");
  setupPane(new PrefsConnecting(world, this), "Connecting");
  setupPane(new PrefsLogging(world, this), "Logging");
  setupPane(new PrefsTimers(world, this), "Timers");
  setupPane(new PrefsChat(world, this), "Chat");
  setupPane(new PrefsOutput(world, this), "Output");
  setupPane(new PrefsMxp(world, this), "MXP");
  setupPane(new PrefsColor(world, this), "ANSI Colour");
  setupPane(new PrefsCustomColor(world, this), "Custom Colour");
  setupPane(new PrefsCommands(world, this), "Commands");
  setupPane(new PrefsAliases(world, this), "Aliases");
  setupPane(new PrefsTriggers(world, this), "Triggers");

  for (QTreeWidgetItemIterator it(ui->settings_tree); *it; ++it)
    (*it)->setExpanded(true);
}

WorldPrefs::~WorldPrefs()
{
  delete ui;
}

void WorldPrefs::setupPane(QWidget *pane, const char *key)
{
  int index = panes.size();
  panes.append(pane);
  ui->contents->addWidget(pane);
  pane->hide();

  QTreeWidgetItem *item =
      ui->settings_tree
          ->findItems(tr(key), Qt::MatchExactly | Qt::MatchRecursive)
          .constFirst();
  item->setData(0, Qt::UserRole, index);

  if (index == 0)
    ui->settings_tree->setCurrentItem(item);
}

void WorldPrefs::on_settings_tree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
  if (current == nullptr)
    return;

  QVariant data = current->data(0, Qt::UserRole);
  if (!data.canConvert<qsizetype>())
    return;

  if (previous != nullptr)
    panes.at(activePane)->hide();

  activePane = data.value<qsizetype>();
  panes.at(activePane)->show();
}
