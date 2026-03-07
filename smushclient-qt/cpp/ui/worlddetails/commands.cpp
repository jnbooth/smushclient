#include "commands.h"
#include "../../fieldconnector.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_commands.h"

PrefsCommands::PrefsCommands(World& world, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::PrefsCommands)
  , world(world)
{
  ui->setupUi(this);

  CONNECT_WORLD(DisplayMyInput);
  CONNECT_WORLD(EchoColour);
  CONNECT_WORLD(EchoBackgroundColour);
  CONNECT_WORLD(KeepCommandsOnSameLine);
  CONNECT_WORLD(EnableSpeedWalk);
  CONNECT_WORLD(SpeedWalkDelay);
  CONNECT_WORLD(SpeedWalkFiller);
  CONNECT_WORLD(EnableCommandStack);
  CONNECT_WORLD(CommandStackDelay);
  ui->SpeedWalkPrefix->setText(
    QChar::fromLatin1(static_cast<char>(world.getSpeedWalkPrefix())));
  ui->CommandStackCharacter->setText(
    QChar::fromLatin1(static_cast<char>(world.getCommandStackCharacter())));
}

PrefsCommands::~PrefsCommands()
{
  delete ui;
}

void
PrefsCommands::on_CommandStackCharacter_textChanged(const QString& character)
{
  if (character.length() != 1) {
    return;
  }
  world.setCommandStackCharacter(character.toUtf8().front());
}

void
PrefsCommands::on_SpeedWalkPrefix_textChanged(const QString& character)
{
  if (character.length() != 1) {
    return;
  }
  world.setSpeedWalkPrefix(character.toUtf8().front());
}
