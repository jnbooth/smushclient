#include "commands.h"
#include "ui_commands.h"
#include "../../fieldconnector.h"

PrefsCommands::PrefsCommands(World *world, QWidget *parent)
    : QWidget(parent), ui(new Ui::PrefsCommands), world(world)
{
  ui->setupUi(this);
  CONNECT_WORLD(DisplayMyInput);
  CONNECT_WORLD(EchoColorsForeground);
  CONNECT_WORLD(EchoColorsBackground);
  CONNECT_WORLD(EnableSpeedWalk);
  CONNECT_WORLD(SpeedWalkPrefix);
  CONNECT_WORLD(SpeedWalkFiller);
  CONNECT_WORLD(SpeedWalkDelay);
  CONNECT_WORLD(EnableCommandStack);
  CONNECT_WORLD(CommandStackCharacter);
  CONNECT_WORLD(InputColorsForeground);
  CONNECT_WORLD(InputColorsBackground);
  CONNECT_WORLD(InputFontSize);
  CONNECT_WORLD(UseDefaultInputFont);
  CONNECT_WORLD(EnableSpamPrevention);
  CONNECT_WORLD(SpamLineCount);
  CONNECT_WORLD(SpamMessage);
  CONNECT_WORLD(AutoRepeat);
  CONNECT_WORLD(LowerCaseTabCompletion);
  CONNECT_WORLD(TranslateGerman);
  CONNECT_WORLD(TranslateBackslashSequences);
  CONNECT_WORLD(KeepCommandsOnSameLine);
  CONNECT_WORLD(NoEchoOff);
  /*
  CONNECT_WORLD(TabCompletionLines);
  CONNECT_WORLD(TabCompletionSpace);
  CONNECT_WORLD(DoubleClickInserts);
  CONNECT_WORLD(DoubleClickSends);
  CONNECT_WORLD(EscapeDeletesInput);
  CONNECT_WORLD(SaveDeletedCommand);
  CONNECT_WORLD(ConfirmBeforeReplacingTyping);
  CONNECT_WORLD(ArrowKeysWrap);
  CONNECT_WORLD(ArrowsChangeHistory);
  CONNECT_WORLD(ArrowRecallsPartial);
  CONNECT_WORLD(AltArrowRrecallsPartial);
  CONNECT_WORLD(CtrlZGoesToEndOfBuffer);
  CONNECT_WORLD(CtrlPGoesToPreviousCommand);
  CONNECT_WORLD(CtrlNGoesToNextCommand);
  */
  CONNECT_WORLD(HistoryLines);

  QFont inputFont(world->getInputFont(), world->getInputFontSize());
  ui->InputFont->setCurrentFont(inputFont);
}

PrefsCommands::~PrefsCommands()
{
  delete ui;
}

// Private methods

void PrefsCommands::on_InputFont_currentFontChanged(const QFont &f)
{
  world->setInputFont(f.family());
}
