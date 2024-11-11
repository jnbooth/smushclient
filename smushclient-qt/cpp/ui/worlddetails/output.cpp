#include "output.h"
#include "ui_output.h"
#include <QtWidgets/QFileDialog>
#include "../../fieldconnector.h"
#include "../../environment.h"
#include "../../scripting/scriptapi.h"

PrefsOutput::PrefsOutput(const World &world, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PrefsOutput),
      audio(),
      player()
{
  ui->setupUi(this);
  CONNECT_WORLD(ShowBold);
  CONNECT_WORLD(ShowItalic);
  CONNECT_WORLD(ShowUnderline);
  CONNECT_WORLD(IndentParas);
  CONNECT_WORLD(Ansi0);
  CONNECT_WORLD(Ansi1);
  CONNECT_WORLD(Ansi2);
  CONNECT_WORLD(Ansi3);
  CONNECT_WORLD(Ansi4);
  CONNECT_WORLD(Ansi5);
  CONNECT_WORLD(Ansi6);
  CONNECT_WORLD(Ansi7);
  CONNECT_WORLD(Ansi8);
  CONNECT_WORLD(Ansi9);
  CONNECT_WORLD(Ansi10);
  CONNECT_WORLD(Ansi11);
  CONNECT_WORLD(Ansi12);
  CONNECT_WORLD(Ansi13);
  CONNECT_WORLD(Ansi14);
  CONNECT_WORLD(Ansi15);
  CONNECT_WORLD(DisplayMyInput);
  CONNECT_WORLD(EchoTextColour);
  CONNECT_WORLD(EchoBackgroundColour);
  CONNECT_WORLD(KeepCommandsOnSameLine);
  CONNECT_WORLD(NewActivitySound);
  player.setAudioOutput(&audio);
}

PrefsOutput::~PrefsOutput()
{
  delete ui;
}

// Private slots

void PrefsOutput::on_NewActivitySound_browse_clicked()
{
  const QString currentFile = ui->NewActivitySound->text();
  const QString path = QFileDialog::getOpenFileName(
      this,
      tr("Select sound file"),
      currentFile.isEmpty() ? QStringLiteral(SOUNDS_DIR) : currentFile);

  if (path.isEmpty())
    return;

  ui->NewActivitySound->setText(makePathRelative(path));
}

void PrefsOutput::on_NewActivitySound_test_clicked()
{
  player.stop();
  player.setSource(QUrl::fromLocalFile(ui->NewActivitySound->text()));
  player.play();
}

void PrefsOutput::on_NewActivitySound_textChanged(const QString &text)
{
  ui->NewActivitySound_test->setEnabled(!text.isEmpty());
}
