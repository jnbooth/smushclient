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
      pane(nullptr),
      settings(settings)
{
  ui->setupUi(this);
  ui->settings_list->setCurrentRow(0);
}

SettingsDialog::~SettingsDialog()
{
  delete ui;
}

// Private methods

QWidget *SettingsDialog::paneForIndex(int n)
{
  switch (n)
  {
  case 0:
    return new SettingsAppearance(settings, &notifier, this);
  default:
    return nullptr;
  }
}

// Private slots

void SettingsDialog::on_settings_list_currentRowChanged(int row)
{
  if (row == -1)
    return;

  if (pane)
    delete pane;

  pane = paneForIndex(row);
  ui->contents->addWidget(pane);
}
