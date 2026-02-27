#pragma once
#include <QtWidgets/QDialog>

namespace Ui {
class PrefsColorMap;
} // namespace Ui

class ColorMapModel;
class World;

class PrefsColorMap : public QDialog
{
  Q_OBJECT

public:
  explicit PrefsColorMap(ColorMapModel& model, QWidget* parent = nullptr);
  ~PrefsColorMap() override;

private slots:
  void on_button_add_clicked();
  void on_table_activated(const QModelIndex& index);
  void on_table_clicked(const QModelIndex& index);

private:
  Ui::PrefsColorMap* ui;
  ColorMapModel& model;
};
