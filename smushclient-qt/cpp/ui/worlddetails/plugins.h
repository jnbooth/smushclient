#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsPlugins;
} // namespace Ui

class PluginModel;
class ScriptApi;
class SmushClient;

class PrefsPlugins : public QWidget
{
  Q_OBJECT

public:
  PrefsPlugins(PluginModel& model, ScriptApi& api, QWidget* parent = nullptr);
  ~PrefsPlugins() override;

private:
  static const QString& settingsKey();

private slots:
  static void onClientError(const QString& error);
  void onPluginOrderChanged();
  void onPluginScriptChanged(size_t pluginIndex);
  void on_button_add_clicked();
  void on_button_reinstall_clicked();
  void on_button_remove_clicked();
  void on_button_showinfo_clicked();
  void on_table_clicked();

private:
  Ui::PrefsPlugins* ui;
  ScriptApi& api;
  PluginModel& model;
};
