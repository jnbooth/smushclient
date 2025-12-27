#pragma once
#include <QtGui/QFont>
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsAppearance;
}

class Settings;
class SettingsNotifier;

class SettingsAppearance : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsAppearance(Settings& settings,
                              SettingsNotifier* notifier,
                              QWidget* parent = nullptr);
  ~SettingsAppearance();

private slots:
  void on_InputBackground_valueChanged(const QColor& color);
  void on_InputFont_currentFontChanged(const QFont& font);
  void on_InputFontSize_valueChanged(int size);
  void on_InputForeground_valueChanged(const QColor& color);
  void on_OutputFont_currentFontChanged(const QFont& font);
  void on_OutputFontSize_valueChanged(int size);
  void on_OutputPadding_valueChanged(double padding);
  void on_OutputLineSpacing_valueChanged(int spacing);

private:
  Ui::SettingsAppearance* ui;
  QFont inputFont;
  SettingsNotifier* notifier;
  QFont outputFont;
  Settings& settings;
};
