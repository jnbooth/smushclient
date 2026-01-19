#include "sound.h"
#include "../../environment.h"
#include "../../fieldconnector.h"
#include "../../settings.h"
#include "ui_sound.h"
#include <QtWidgets/QFileDialog>

// Public methods

SettingsSound::SettingsSound(Settings& settings, QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::SettingsSound)
{
  ui->setupUi(this);
  CONNECT_SETTINGS(BellSound);
}

SettingsSound::~SettingsSound()
{
  delete ui;
}

// Private slots

void
SettingsSound::on_BellSound_browse_clicked()
{
  const QString currentFile = ui->BellSound->text();
  const QString path = QFileDialog::getOpenFileName(
    this,
    tr("Select sound file"),
    currentFile.isEmpty() ? QStringLiteral(SOUNDS_DIR) : currentFile);

  if (path.isEmpty()) {
    return;
  }

  ui->BellSound->setText(makePathRelative(path));
}

void
SettingsSound::on_BellSound_test_clicked()
{
  audio.play();
}

void
SettingsSound::on_BellSound_textChanged(const QString& text)
{
  ui->BellSound_test->setEnabled(!text.isEmpty());
  ui->SoundError->setText(audio.setFile(text));
}
