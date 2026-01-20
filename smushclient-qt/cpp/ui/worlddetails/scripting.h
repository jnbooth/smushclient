#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsScripting;
} // namespace Ui

class World;

class PrefsScripting : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsScripting(World& world, QWidget* parent = nullptr);
  ~PrefsScripting() override;

private:
  QString defaultScriptPath() const;

private slots:
  void on_WorldScript_browse_clicked();
  void on_WorldScript_create_clicked();
  void on_WorldScript_edit_clicked();
  void on_WorldScript_textChanged(const QString& value);

private:
  Ui::PrefsScripting* ui;
  World& world;
};
