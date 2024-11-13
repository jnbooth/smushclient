#pragma once
#include <QtWidgets/QWidget>
#include "../../bridge/viewbuilder.h"

namespace Ui
{
  class PrefsPlugins;
}

class ScriptApi;
class SmushClient;

class PrefsPlugins : public QWidget
{
  Q_OBJECT

public:
  PrefsPlugins(SmushClient &client, ScriptApi *api, QWidget *parent = nullptr);
  ~PrefsPlugins();

private:
  Ui::PrefsPlugins *ui;
  ScriptApi *api;
  ModelBuilder *builder;
  SmushClient &client;

private:
  QString activePluginId();
  void buildTable();
  void initPlugins();
  constexpr QStandardItemModel *model() const noexcept { return builder->model(); }

private slots:
  void on_button_add_clicked();
  void on_button_reinstall_clicked();
  void on_button_remove_clicked();
};
