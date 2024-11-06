#pragma once
#include <QtGui/QFont>
#include <QtWidgets/QWidget>
#include "../../settings.h"

namespace Ui
{
  class SettingsAppearance;
}

class SettingsNotifier;

class SettingsAppearance : public QWidget
{
  Q_OBJECT

public:
  explicit SettingsAppearance(Settings &settings, SettingsNotifier *notifier, QWidget *parent = nullptr);
  ~SettingsAppearance();

private:
  Ui::SettingsAppearance *ui;
  QFont inputFont;
  SettingsNotifier *notifier;
  QFont outputFont;
  Settings &settings;

private slots:
  void on_inputBackground_valueChanged(const QColor &color);
  void on_inputFont_currentFontChanged(const QFont &font);
  void on_inputFontSize_valueChanged(int size);
  void on_inputForeground_valueChanged(const QColor &color);
  void on_outputFont_currentFontChanged(const QFont &font);
  void on_outputFontSize_valueChanged(int size);
};
