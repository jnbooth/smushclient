#include "settings.h"
#include "ui_settings.h"
#include "appearance.h"
#include "history.h"
#include "notifier.h"
#include "sound.h"
#include "startup.h"
#include "../worldtab.h"
#include "../ui_worldtab.h"
#include "../../settings.h"

static SettingsNotifier notifier;

// Static methods

void SettingsDialog::connect(WorldTab *tab)
{
  tab->connect(&notifier, &SettingsNotifier::inputBackgroundChanged, tab, &WorldTab::onInputBackgroundChanged);
  tab->connect(&notifier, &SettingsNotifier::inputForegroundChanged, tab, &WorldTab::onInputForegroundChanged);
  tab->connect(&notifier, &SettingsNotifier::inputFontChanged, tab, &WorldTab::onInputFontChanged);
  tab->connect(&notifier, &SettingsNotifier::outputFontChanged, tab, &WorldTab::onOutputFontChanged);
  tab->connect(&notifier, &SettingsNotifier::inputHistoryLimitChanged, tab->ui->input, &MudInput::setMaxLogSize);
  tab->connect(&notifier, &SettingsNotifier::outputLimitChanged, tab->ui->output, &MudBrowser::setMaximumBlockCount);
  tab->connect(&notifier, &SettingsNotifier::outputPaddingChanged, tab, &WorldTab::onOutputPaddingChanged);
  tab->connect(&notifier, &SettingsNotifier::outputBlockFormatChanged, tab, &WorldTab::onOutputBlockFormatChanged);
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
  case 1:
    return new SettingsHistory(settings, &notifier, this);
  case 2:
    return new SettingsSound(settings, this);
  case 3:
    return new SettingsStartup(settings, this);
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
