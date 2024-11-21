#pragma once
#include <QtWidgets/QDialog>

namespace Ui
{
  class PluginPopup;
}

class PluginDetails;
class SmushClient;

class PluginPopup : public QDialog
{
  Q_OBJECT

public:
  PluginPopup(const PluginDetails &plugin, QWidget *parent = nullptr);
  ~PluginPopup();

private:
  Ui::PluginPopup *ui;
};
