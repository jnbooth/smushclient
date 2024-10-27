#include "commands.h"
#include "ui_commands.h"
#include "../../fieldconnector.h"

PrefsCommands::PrefsCommands(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsCommands)
{
  ui->setupUi(this);
  CONNECT_WORLD(DisplayMyInput);
  CONNECT_WORLD(EchoTextColour);
  CONNECT_WORLD(EchoBackgroundColour);
  CONNECT_WORLD(EnableSpeedWalk);
  CONNECT_WORLD(SpeedWalkPrefix);
  CONNECT_WORLD(SpeedWalkFiller);
  CONNECT_WORLD(SpeedWalkDelay);
  CONNECT_WORLD(EnableCommandStack);
  CONNECT_WORLD(CommandStackCharacter);
  CONNECT_WORLD(EnableSpamPrevention);
  CONNECT_WORLD(SpamLineCount);
  CONNECT_WORLD(SpamMessage);
  CONNECT_WORLD(KeepCommandsOnSameLine);
  CONNECT_WORLD(NoEchoOff);
}

PrefsCommands::~PrefsCommands()
{
  delete ui;
}
