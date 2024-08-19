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

WorldPrefs::WorldPrefs(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WorldPrefs)
    , panes(new QList<QWidget *>)
    , activePane(NULL)
{
    ui->setupUi(this);
    panes->reserve(12);
    setupPane(new PrefsAddress(this), "IP address");
    setupPane(new PrefsConnecting(this), "Connecting");
    setupPane(new PrefsLogging(this), "Logging");
    setupPane(new PrefsTimers(this), "Timers");
    setupPane(new PrefsChat(this), "Chat");
    setupPane(new PrefsOutput(this), "Output");
    setupPane(new PrefsMxp(this), "MXP/Pueblo");
    setupPane(new PrefsColor(this), "ANSI Colour");
    setupPane(new PrefsCustomColor(this), "Custom Colour");
    setupPane(new PrefsCommands(this), "Commands");
    setupPane(new PrefsAliases(this), "Aliases");
    setupPane(new PrefsTriggers(this), "Triggers");

    for (QTreeWidgetItemIterator it(ui->settings_tree); *it; ++it) {
      (*it)->setExpanded(true);
    }
}

WorldPrefs::~WorldPrefs()
{
    delete ui;
}

void WorldPrefs::setupPane(QWidget *pane, const char *key) {
  int index = panes->size();
  panes->append(pane);
  ui->contents->addWidget(pane);
  pane->hide();

  QTreeWidgetItem *item = ui
    ->settings_tree
    ->findItems(tr(key), Qt::MatchExactly | Qt::MatchRecursive)
    .constFirst();
  item->setData(0, Qt::UserRole, index);

  if (index == 0) {
    ui->settings_tree->setCurrentItem(item);
  }
}

void WorldPrefs::on_settings_tree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
  if (current == NULL) {
    return;
  }
  QVariant data = current->data(0, Qt::UserRole);
  if (!data.canConvert<qsizetype>()) {
    return;
  }
  if (activePane != NULL) {
    activePane->hide();
  }
  activePane = panes->at(data.value<qsizetype>());
  activePane->show();
}

