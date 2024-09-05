#include "logging.h"
#include "ui_logging.h"
#include "../../fieldconnector.h"
#include <QtWidgets/QButtonGroup>

PrefsLogging::PrefsLogging(World *world, QWidget *parent)
    : QWidget(parent), ui(new Ui::PrefsLogging), world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(LogOutput);
  CONNECT_WORLD(LogInput);
  CONNECT_WORLD(LogNotes);
  CONNECT_WORLD(AutoLogFileName);
  CONNECT_WORLD(LogPreambleOutput);
  CONNECT_WORLD(LogPreambleInput);
  CONNECT_WORLD(LogPreambleNotes);
  CONNECT_WORLD(LogPreambleOutput);
  CONNECT_WORLD(LogPostambleOutput);
  CONNECT_WORLD(LogPostambleInput);
  CONNECT_WORLD(LogPostambleNotes);

  ui->LogFilePreamble->setPlainText(world->getLogFilePreamble());
  ui->LogFilePostamble->setPlainText(world->getLogFilePostamble());

  QButtonGroup *logFormatGroup = new QButtonGroup(this);
  logFormatGroup->setExclusive(true);
  logFormatGroup->addButton(ui->LogFormat_Text);
  logFormatGroup->setId(ui->LogFormat_Text, (int)LogFormat::Text);
  logFormatGroup->addButton(ui->LogFormat_Html);
  logFormatGroup->setId(ui->LogFormat_Html, (int)LogFormat::Html);
  logFormatGroup->addButton(ui->LogFormat_Raw);
  logFormatGroup->setId(ui->LogFormat_Raw, (int)LogFormat::Raw);
  logFormatGroup->button((int)world->getLogFormat())->setChecked(true);
  connect(logFormatGroup, &QButtonGroup::idClicked, this, &PrefsLogging::on_LogFormatIdClicked);

  QButtonGroup *logModeGroup = new QButtonGroup(this);
  logModeGroup->setExclusive(true);
  logModeGroup->addButton(ui->LogMode_Append);
  logModeGroup->setId(ui->LogMode_Append, (int)LogMode::Append);
  logModeGroup->addButton(ui->LogMode_Overwrite);
  logModeGroup->setId(ui->LogMode_Overwrite, (int)LogMode::Overwrite);
  logModeGroup->button((int)world->getLogMode())->setChecked(true);
  connect(logModeGroup, &QButtonGroup::idClicked, this, &PrefsLogging::on_LogModeIdClicked);
}

PrefsLogging::~PrefsLogging()
{
  delete ui;
}

// Slots

void PrefsLogging::on_LogFilePreamble_textChanged()
{
  world->setLogFilePreamble(ui->LogFilePreamble->toPlainText());
}

void PrefsLogging::on_LogFilePostamble_textChanged()
{
  world->setLogFilePostamble(ui->LogFilePostamble->toPlainText());
}

void PrefsLogging::on_LogFormatIdClicked(int id)
{
  world->setLogFormat((LogFormat)id);
}

void PrefsLogging::on_LogModeIdClicked(int id)
{
  world->setLogMode((LogMode)id);
}
