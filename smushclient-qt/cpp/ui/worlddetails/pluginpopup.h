#pragma once
#include <QtWidgets/QDialog>

namespace Ui {
class PluginPopup;
} // namespace Ui

class PluginDetails;
class SmushClient;

class PluginPopup : public QDialog
{
  Q_OBJECT

public:
  explicit PluginPopup(const PluginDetails& plugin, QWidget* parent = nullptr);
  ~PluginPopup() override;

private:
  Ui::PluginPopup* ui;
};
