#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsCommands;
} // namespace Ui

class World;

class PrefsCommands : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsCommands(World& world, QWidget* parent = nullptr);
  ~PrefsCommands() override;

private slots:
  void on_CommandStackCharacter_textChanged(const QString& character);
  void on_SpeedWalkPrefix_textChanged(const QString& character);

private:
  Ui::PrefsCommands* ui;
  World& world;
};
