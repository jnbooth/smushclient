#include "mxp.h"
#include "ui_mxp.h"
#include "../../fieldconnector.h"

PrefsMxp::PrefsMxp(const World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsMxp)
{
  ui->setupUi(this);
  CONNECT_WORLD(UseMxp);
  CONNECT_WORLD(HyperlinkColor);
  CONNECT_WORLD(UseCustomLinkColor);
  CONNECT_WORLD(MudCanChangeLinkColor);
  CONNECT_WORLD(UnderlineHyperlinks);
  CONNECT_WORLD(MudCanRemoveUnderline);
  CONNECT_WORLD(HyperlinkAddsToCommandHistory);
  CONNECT_WORLD(EchoHyperlinkInOutputWindow);
  CONNECT_WORLD(IgnoreMxpColorChanges);
  CONNECT_WORLD(SendMxpAfkResponse);
}

PrefsMxp::~PrefsMxp()
{
  delete ui;
}
