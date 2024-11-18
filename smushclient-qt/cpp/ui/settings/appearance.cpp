#include "appearance.h"
#include "ui_appearance.h"
#include "settings.h"
#include "notifier.h"
#include "../../fieldconnector.h"

// Public methods

SettingsAppearance::SettingsAppearance(Settings &settings, SettingsNotifier *notifier, QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingsAppearance),
      inputFont(settings.getInputFont()),
      notifier(notifier),
      outputFont(settings.getOutputFont()),
      settings(settings)
{
  ui->setupUi(this);
  ui->InputFont->setCurrentFont(inputFont);
  ui->InputFontSize->setValue(inputFont.pointSize());
  ui->OutputFont->setCurrentFont(outputFont);
  ui->OutputFontSize->setValue(outputFont.pointSize());

  CONNECT_SETTINGS(InputBackground);
  CONNECT_SETTINGS(InputForeground);
}

SettingsAppearance::~SettingsAppearance()
{
  delete ui;
}

// Private slots

void SettingsAppearance::on_InputBackground_valueChanged(const QColor &color)
{
  emit notifier->inputBackgroundChanged(color);
}

void SettingsAppearance::on_InputFont_currentFontChanged(const QFont &font)
{
  if (inputFont == font)
    return;
  const int pointSize = inputFont.pointSize();
  inputFont = font;
  inputFont.setPointSize(pointSize);
  settings.setInputFont(inputFont);
  emit notifier->inputFontChanged(inputFont);
}

void SettingsAppearance::on_InputFontSize_valueChanged(int size)
{
  if (inputFont.pointSize() == size)
    return;
  inputFont.setPointSize(size);
  settings.setInputFont(inputFont);
  emit notifier->inputFontChanged(inputFont);
}

void SettingsAppearance::on_InputForeground_valueChanged(const QColor &color)
{
  emit notifier->inputForegroundChanged(color);
}

void SettingsAppearance::on_OutputFont_currentFontChanged(const QFont &font)
{
  if (outputFont == font)
    return;
  const int pointSize = outputFont.pointSize();
  outputFont = font;
  outputFont.setPointSize(pointSize);
  settings.setOutputFont(outputFont);
  emit notifier->outputFontChanged(outputFont);
}

void SettingsAppearance::on_OutputFontSize_valueChanged(int size)
{
  if (outputFont.pointSize() == size)
    return;
  outputFont.setPointSize(size);
  settings.setOutputFont(outputFont);
  emit notifier->outputFontChanged(outputFont);
}
