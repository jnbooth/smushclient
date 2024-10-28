#include "commands.h"
#include "ui_commands.h"
#include "../../fieldconnector.h"

PrefsCommands::PrefsCommands(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsCommands),
      world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(DisplayMyInput);
  CONNECT_WORLD(EchoTextColour);
  CONNECT_WORLD(EchoBackgroundColour);
  CONNECT_WORLD(EnableSpeedWalk);
  CONNECT_WORLD(SpeedWalkPrefix);
  CONNECT_WORLD(SpeedWalkFiller);
  CONNECT_WORLD(CommandQueueDelay);
  CONNECT_WORLD(EnableCommandStack);
  CONNECT_WORLD(EnableSpamPrevention);
  CONNECT_WORLD(SpamLineCount);
  CONNECT_WORLD(SpamMessage);
  CONNECT_WORLD(KeepCommandsOnSameLine);
  CONNECT_WORLD(NoEchoOff);
  ui->CommandStackCharacter->setText(QChar(world.getCommandStackCharacter()));
}

PrefsCommands::~PrefsCommands()
{
  delete ui;
}

void PrefsCommands::on_CommandStackCharacter_textChanged(const QString &character)
{
  if (character.length() != 1)
    return;
  world.setCommandStackCharacter(character.front().unicode());
}
