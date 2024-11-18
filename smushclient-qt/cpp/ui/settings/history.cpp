#include "history.h"
#include "ui_history.h"
#include "../../fieldconnector.h"
#include "notifier.h"

int getLimit(QCheckBox *checkbox, QSpinBox *spinbox)
{
  return checkbox->isChecked() ? spinbox->value() : -1;
}

// Public methods

SettingsHistory::SettingsHistory(Settings &settings, SettingsNotifier *notifier, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::History),
      notifier(notifier),
      settings(settings)
{
  ui->setupUi(this);
  CONNECT_SETTINGS(InputHistoryLimit);
  CONNECT_SETTINGS(InputHistoryLines);
  CONNECT_SETTINGS(OutputLimit);
  CONNECT_SETTINGS(OutputLines);
  CONNECT_SETTINGS(OutputHistoryEnabled);
  CONNECT_SETTINGS(OutputHistoryLimit);
  CONNECT_SETTINGS(OutputHistoryLines);

  connect(ui->InputHistoryLimit, &QCheckBox::toggled, this, &SettingsHistory::updateInputHistoryLimit);
  connect(ui->InputHistoryLines, &QSpinBox::valueChanged, this, &SettingsHistory::updateInputHistoryLimit);
  connect(ui->OutputLimit, &QCheckBox::toggled, this, &SettingsHistory::updateOutputLimit);
  connect(ui->OutputLines, &QSpinBox::valueChanged, this, &SettingsHistory::updateOutputLimit);
  connect(ui->OutputHistoryLimit, &QCheckBox::toggled, this, &SettingsHistory::updateOutputHistoryLimit);
  connect(ui->OutputHistoryLines, &QSpinBox::valueChanged, this, &SettingsHistory::updateOutputHistoryLimit);
}

SettingsHistory::~SettingsHistory()
{
  delete ui;
}

// Private slots

void SettingsHistory::updateInputHistoryLimit()
{
  const int limit = getLimit(ui->InputHistoryLimit, ui->InputHistoryLines);
  emit notifier->inputHistoryLimitChanged(limit);
}

void SettingsHistory::updateOutputLimit()
{
  const int limit = getLimit(ui->OutputLimit, ui->OutputLines);
  emit notifier->outputLimitChanged(limit);
}

void SettingsHistory::updateOutputHistoryLimit()
{
  const int limit = getLimit(ui->OutputHistoryLimit, ui->OutputHistoryLines);
  emit notifier->outputHistoryLimitChanged(limit);
}
