#include "logging.h"
#include "ui_logging.h"
#include "specialhelp.h"
#include "../../enumbuttongroup.h"
#include "../../fieldconnector.h"
#include "smushclient_qt/src/bridge.cxxqt.h"

PrefsLogging::PrefsLogging(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsLogging),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(LogOutput);
  CONNECT_WORLD(LogInput);
  CONNECT_WORLD(LogNotes);
  CONNECT_WORLD(AutoLogFileName);
  CONNECT_WORLD(LogPreambleOutput);
  CONNECT_WORLD(LogPostambleOutput);
  CONNECT_WORLD(LogPreambleInput);
  CONNECT_WORLD(LogPostambleInput);
  CONNECT_WORLD(LogPreambleNotes);
  CONNECT_WORLD(LogPostambleNotes);

  ui->LogFilePreamble->setPlainText(world.getLogFilePreamble());
  ui->LogFilePostamble->setPlainText(world.getLogFilePostamble());

  EnumButtonGroup(this, world.getLogFormat(), &PrefsLogging::on_LogFormatChanged)
      .addButton(ui->LogFormat_Text, LogFormat::Text)
      .addButton(ui->LogFormat_Html, LogFormat::Html)
      .addButton(ui->LogFormat_Raw, LogFormat::Raw);

  EnumButtonGroup(this, world.getLogMode(), &PrefsLogging::on_LogModeChanged)
      .addButton(ui->LogMode_Append, LogMode::Append)
      .addButton(ui->LogMode_Overwrite, LogMode::Overwrite);
}

PrefsLogging::~PrefsLogging()
{
  delete ui;
}

// Private methods

void PrefsLogging::on_LogFilePreamble_textChanged()
{
  world.setLogFilePreamble(ui->LogFilePreamble->toPlainText());
}

void PrefsLogging::on_LogFilePostamble_textChanged()
{
  world.setLogFilePostamble(ui->LogFilePostamble->toPlainText());
}

void PrefsLogging::on_LogFormatChanged(LogFormat value)
{
  world.setLogFormat(value);
}

void PrefsLogging::on_LogModeChanged(LogMode value)
{
  world.setLogMode(value);
}

void PrefsLogging::on_view_special_clicked()
{
  SpecialHelp(this).exec();
}
