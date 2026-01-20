#include "mud.h"
#include "../../bytes.h"
#include "../../fieldconnector.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_mud.h"

PrefsMud::PrefsMud(World& world, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::PrefsMud)
  , world(world)
{
  ui->setupUi(this);

  CONNECT_WORLD(UseMxp);
  CONNECT_WORLD(IgnoreMxpColourChanges);
  CONNECT_WORLD(UseCustomLinkColour);
  CONNECT_WORLD(HyperlinkColour);
  CONNECT_WORLD(MudCanChangeLinkColour);
  CONNECT_WORLD(UnderlineHyperlinks);
  CONNECT_WORLD(HyperlinkAddsToCommandHistory);
  CONNECT_WORLD(EchoHyperlinkInOutputWindow);
  CONNECT_WORLD(TerminalIdentification);
  CONNECT_WORLD(DisableCompression);
  CONNECT_WORLD(Naws);
  CONNECT_WORLD(CarriageReturnClearsLine);
  CONNECT_WORLD(Utf8);
  CONNECT_WORLD(ConvertGaToNewline);
  CONNECT_WORLD(NoEchoOff);
  CONNECT_WORLD(EnableCommandStack);
  ui->CommandStackCharacter->setText(
    bytes::qChar(world.getCommandStackCharacter()));
}

PrefsMud::~PrefsMud()
{
  delete ui;
}

void
PrefsMud::on_CommandStackCharacter_textChanged(const QString& character)
{
  if (character.length() != 1) {
    return;
  }
  world.setCommandStackCharacter(character.toUtf8().front());
}

void
PrefsMud::on_UseMxp_currentIndexChanged(int index)
{
  const UseMxp value = static_cast<UseMxp>(index);
  const bool enableMxp = value != UseMxp::Never;
  ui->IgnoreMxpColourChanges->setEnabled(enableMxp);
  ui->UseCustomLinkColour->setEnabled(enableMxp);
  ui->MudCanChangeLinkColour->setEnabled(enableMxp);
  ui->UnderlineHyperlinks->setEnabled(enableMxp);
  ui->HyperlinkAddsToCommandHistory->setEnabled(enableMxp);
  ui->EchoHyperlinkInOutputWindow->setEnabled(enableMxp);
}
