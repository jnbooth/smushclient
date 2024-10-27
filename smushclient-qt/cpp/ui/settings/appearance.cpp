#include "appearance.h"
#include "ui_appearance.h"
#include "../settings.h"

#undef emit
#define emit qobject_cast<SettingsDialog *>(parent())->

// Public methods

SettingsAppearance::SettingsAppearance(Settings &settings, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingsAppearance),
      inputFont(settings.inputFont()),
      outputFont(settings.outputFont()),
      settings(settings)
{
  ui->setupUi(this);
  ui->inputBackground->setValue(settings.inputBackground());
  ui->inputForeground->setValue(settings.inputForeground());
}

SettingsAppearance::~SettingsAppearance()
{
  delete ui;
}

// Private slots

void SettingsAppearance::on_inputBackground_valueChanged(const QColor &color)
{
  settings.setInputBackground(color);
  emit inputBackgroundChanged(color);
}

void SettingsAppearance::on_inputFont_currentFontChanged(const QFont &font)
{
  if (inputFont == font)
    return;
  const int pointSize = inputFont.pointSize();
  inputFont = font;
  inputFont.setPointSize(pointSize);
  settings.setInputFont(inputFont);
  emit inputFontChanged(inputFont);
}

void SettingsAppearance::on_inputFontSize_valueChanged(int size)
{
  if (inputFont.pointSize() == size)
    return;
  inputFont.setPointSize(size);
  settings.setInputFont(inputFont);
  emit inputFontChanged(inputFont);
}

void SettingsAppearance::on_inputForeground_valueChanged(const QColor &color)
{
  settings.setInputForeground(color);
  emit inputForegroundChanged(color);
}

void SettingsAppearance::on_outputFont_currentFontChanged(const QFont &font)
{
  if (outputFont == font)
    return;
  const int pointSize = outputFont.pointSize();
  outputFont = font;
  outputFont.setPointSize(pointSize);
  settings.setOutputFont(outputFont);
  emit outputFontChanged(outputFont);
}

void SettingsAppearance::on_outputFontSize_valueChanged(int size)
{
  if (outputFont.pointSize() == size)
    return;
  outputFont.setPointSize(size);
  settings.setOutputFont(outputFont);
  emit outputFontChanged(outputFont);
}
