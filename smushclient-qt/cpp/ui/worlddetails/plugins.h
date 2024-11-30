#pragma once
#include <QtWidgets/QWidget>

namespace Ui
{
  class PrefsPlugins;
}

class PluginModel;
class ScriptApi;
class SmushClient;

class PrefsPlugins : public QWidget
{
  Q_OBJECT

public:
  PrefsPlugins(PluginModel *model, ScriptApi *api, QWidget *parent = nullptr);
  ~PrefsPlugins();

private:
  Ui::PrefsPlugins *ui;
  ScriptApi *api;

private:
  PluginModel *model;

private:
  static const QString &settingsKey();

private slots:
  void onClientError(const QString &error);
  void onPluginOrderChanged();
  void onPluginScriptChanged(size_t pluginIndex);
  void on_button_add_clicked();
  void on_button_reinstall_clicked();
  void on_button_remove_clicked();
  void on_button_showinfo_clicked();
  void on_table_clicked();
};
