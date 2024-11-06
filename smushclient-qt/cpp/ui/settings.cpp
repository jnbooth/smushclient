#include "settings.h"
#include "ui_settings.h"
#include "settings/appearance.h"
#include "settings/notifier.h"
#include "worldtab.h"

static SettingsNotifier notifier;

// Static methods

void SettingsDialog::connect(WorldTab *tab)
{
  tab->connect(&notifier, &SettingsNotifier::inputBackgroundChanged, tab, &WorldTab::onInputBackgroundChanged);
  tab->connect(&notifier, &SettingsNotifier::inputForegroundChanged, tab, &WorldTab::onInputForegroundChanged);
  tab->connect(&notifier, &SettingsNotifier::inputFontChanged, tab, &WorldTab::onInputFontChanged);
  tab->connect(&notifier, &SettingsNotifier::outputFontChanged, tab, &WorldTab::onOutputFontChanged);
}

// Public methods

SettingsDialog::SettingsDialog(Settings &settings, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::SettingsDialog),
      panes(),
      activePane(0)
{
  ui->setupUi(this);
  panes.reserve(1);
  setupPane(new SettingsAppearance(settings, &notifier, this), "Appearance");
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

// Private methods

void SettingsDialog::setupPane(QWidget *pane, const char *key)
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

void SettingsDialog::on_settings_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
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
