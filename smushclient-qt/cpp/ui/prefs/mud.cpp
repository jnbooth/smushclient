#include "mud.h"
#include "ui_mud.h"
#include "../../fieldconnector.h"

PrefsMud::PrefsMud(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsMud)
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
}

PrefsMud::~PrefsMud()
{
  delete ui;
}
