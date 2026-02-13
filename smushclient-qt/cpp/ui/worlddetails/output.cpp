#include "output.h"
#include "../../environment.h"
#include "../../fieldconnector.h"
#include "smushclient_qt/src/ffi/util.cxx.h"
#include "smushclient_qt/src/ffi/world.cxxqt.h"
#include "ui_output.h"
#include <QtWidgets/QFileDialog>

using std::array;

PrefsOutput::PrefsOutput(const World& world, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::PrefsOutput)
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
  CONNECT_WORLD(EchoColour);
  CONNECT_WORLD(EchoBackgroundColour);
  CONNECT_WORLD(KeepCommandsOnSameLine);
  CONNECT_WORLD(NewActivitySound);
}

PrefsOutput::~PrefsOutput()
{
  delete ui;
}

// Private slots

void
PrefsOutput::on_NewActivitySound_browse_clicked()
{
  const QString currentFile = ui->NewActivitySound->text();
  const QString path = QFileDialog::getOpenFileName(
    this,
    tr("Select sound file"),
    currentFile.isEmpty() ? QStringLiteral(SOUNDS_DIR) : currentFile);

  if (path.isEmpty()) {
    return;
  }

  ui->NewActivitySound->setText(makePathRelative(path));
}

void
PrefsOutput::on_NewActivitySound_test_clicked()
{
  audio.play();
}

void
PrefsOutput::on_NewActivitySound_textChanged(const QString& text)
{
  ui->NewActivitySound_test->setEnabled(!text.isEmpty());
  ui->SoundError->setText(audio.setFile(text));
}

void
PrefsOutput::on_reset_clicked()
{
  const QVector<QColor> ansiColors = ffi::util::ansi16();
  array<ColorPickerButton*, 16> colorPickers{
    ui->Ansi0,  ui->Ansi1,  ui->Ansi2,  ui->Ansi3, ui->Ansi4,  ui->Ansi5,
    ui->Ansi6,  ui->Ansi7,  ui->Ansi8,  ui->Ansi9, ui->Ansi10, ui->Ansi11,
    ui->Ansi12, ui->Ansi13, ui->Ansi14, ui->Ansi15
  };
  auto ansi = ansiColors.cbegin();
  for (ColorPickerButton* picker : colorPickers) {
    picker->setValue(*ansi);
    ++ansi;
  }
}
