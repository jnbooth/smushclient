#include "output.h"
#include "ui_output.h"
#include "../../fieldconnector.h"

PrefsOutput::PrefsOutput(World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsOutput)
{
  ui->setupUi(this);

  CONNECT_WORLD(ShowBold);
  CONNECT_WORLD(ShowItalic);
  CONNECT_WORLD(ShowUnderline);
  CONNECT_WORLD(NewActivitySound);
  CONNECT_WORLD(DisableCompression);
  CONNECT_WORLD(IndentParas);
  CONNECT_WORLD(Naws);
  CONNECT_WORLD(CarriageReturnClearsLine);
  CONNECT_WORLD(Utf8);
  CONNECT_WORLD(ConvertGaToNewline);
  CONNECT_WORLD(TerminalIdentification);
}

PrefsOutput::~PrefsOutput()
{
  delete ui;
}
