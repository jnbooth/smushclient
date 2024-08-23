#include "output.h"
#include "ui_output.h"

PrefsOutput::PrefsOutput(World *world, QWidget *parent)
    : AbstractPrefsPane(parent), ui(new Ui::PrefsOutput), world(world)
{
  ui->setupUi(this);

  CONNECT_WORLD(BeepSound);
  CONNECT_WORLD(PixelOffset);
  CONNECT_WORLD(LineSpacing);
  CONNECT_WORLD(OutputFontSize);
  CONNECT_WORLD(UseDefaultOutputFont);
  CONNECT_WORLD(ShowBold);
  CONNECT_WORLD(ShowItalic);
  CONNECT_WORLD(ShowUnderline);
  CONNECT_WORLD(NewActivitySound);
  CONNECT_WORLD(MaxOutputLines);
  CONNECT_WORLD(WrapColumn);
  CONNECT_WORLD(LineInformation);
  CONNECT_WORLD(StartPaused);
  CONNECT_WORLD(AutoPause);
  CONNECT_WORLD(UnpauseOnSend);
  CONNECT_WORLD(FlashTaskbarIcon);
  CONNECT_WORLD(DisableCompression);
  CONNECT_WORLD(IndentParas);
  CONNECT_WORLD(Naws);
  CONNECT_WORLD(CarriageReturnClearsLine);
  CONNECT_WORLD(Utf8);
  CONNECT_WORLD(AutoWrapWindowWidth);
  CONNECT_WORLD(ShowConnectDisconnect);
  CONNECT_WORLD(CopySelectionToClipboard);
  CONNECT_WORLD(AutoCopyToClipboardInHtml);
  CONNECT_WORLD(ConvertGaToNewline);
  CONNECT_WORLD(TerminalIdentification);

  QFont outputFont(world->getOutputFont(), world->getOutputFontSize());
  ui->OutputFont->setCurrentFont(outputFont);
}

PrefsOutput::~PrefsOutput()
{
  delete ui;
}

void PrefsOutput::on_OutputFont_currentFontChanged(const QFont &f)
{
  world->setOutputFont(f.family());
}
