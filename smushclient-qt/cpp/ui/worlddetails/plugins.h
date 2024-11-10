#pragma once
#include <QtWidgets/QWidget>

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
  SmushClient &client;

  void buildTable();
  void initPlugins();

private slots:
  void on_button_add_clicked();
  void on_button_reinstall_clicked();
  void on_button_remove_clicked();
};
