#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsLogging;
} // namespace Ui

class World;
enum class LogFormat;
enum class LogMode;

class PrefsLogging : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsLogging(World& world, QWidget* parent = nullptr);
  ~PrefsLogging() override;

private slots:
  void on_AutoLogFileName_browse_clicked();
  void on_LogFilePostamble_textChanged();
  void on_LogFilePreamble_textChanged();
  void on_LogFormatChanged(LogFormat value);
  void on_LogModeChanged(LogMode value);
  void on_view_special_clicked();

private:
  Ui::PrefsLogging* ui;
  World& world;
};
