#include "appearance.h"
#include "../../fieldconnector.h"
#include "../../settings.h"
#include "notifier.h"
#include "settings.h"
#include "ui_appearance.h"

// Public methods

SettingsAppearance::SettingsAppearance(Settings& settings,
                                       SettingsNotifier* notifier,
                                       QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::SettingsAppearance)
  , inputFont(settings.getInputFont())
  , notifier(notifier)
  , outputFont(settings.getOutputFont())
  , settings(settings)
{
  ui->setupUi(this);

  ui->InputBackground->setValue(settings.getInputBackground());
  ui->InputFont->setCurrentFont(inputFont);
  ui->InputFontSize->setValue(inputFont.pointSize());
  ui->InputForeground->setValue(settings.getInputForeground());

  ui->OutputFont->setCurrentFont(outputFont);
  ui->OutputFontSize->setValue(outputFont.pointSize());
  ui->OutputLineSpacing->setValue(settings.getOutputLineSpacing());
  ui->OutputPadding->setValue(settings.getOutputPadding());
}

SettingsAppearance::~SettingsAppearance()
{
  delete ui;
}

// Private slots

void
SettingsAppearance::on_InputBackground_valueChanged(const QColor& color)
{
  settings.setInputBackground(color);
  emit notifier->inputBackgroundChanged(color);
}

void
SettingsAppearance::on_InputFont_currentFontChanged(const QFont& font)
{
  if (inputFont == font)
    return;
  const int pointSize = inputFont.pointSize();
  inputFont = font;
  inputFont.setPointSize(pointSize);
  settings.setInputFont(inputFont);
  emit notifier->inputFontChanged(inputFont);
}

void
SettingsAppearance::on_InputFontSize_valueChanged(int size)
{
  if (inputFont.pointSize() == size)
    return;
  inputFont.setPointSize(size);
  settings.setInputFont(inputFont);
  emit notifier->inputFontChanged(inputFont);
}

void
SettingsAppearance::on_InputForeground_valueChanged(const QColor& color)
{
  settings.setInputForeground(color);
  emit notifier->inputForegroundChanged(color);
}

void
SettingsAppearance::on_OutputFont_currentFontChanged(const QFont& font)
{
  if (outputFont == font)
    return;
  const int pointSize = outputFont.pointSize();
  outputFont = font;
  outputFont.setPointSize(pointSize);
  settings.setOutputFont(outputFont);
  emit notifier->outputFontChanged(outputFont);
}

void
SettingsAppearance::on_OutputFontSize_valueChanged(int size)
{
  if (outputFont.pointSize() == size)
    return;
  outputFont.setPointSize(size);
  settings.setOutputFont(outputFont);
  emit notifier->outputFontChanged(outputFont);
}

void
SettingsAppearance::on_OutputPadding_valueChanged(double padding)
{
  settings.setOutputPadding(padding);
  emit notifier->outputPaddingChanged(padding);
}

void
SettingsAppearance::on_OutputLineSpacing_valueChanged(int spacing)
{
  settings.setOutputLineSpacing(spacing);
  const QTextBlockFormat fmt = settings.getOutputBlockFormat();
  emit notifier->outputBlockFormatChanged(fmt);
}
