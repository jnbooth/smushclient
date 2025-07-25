#pragma once
#include <QtCore/QAbstractItemModel>
#include <QtWidgets/QDialog>

namespace Ui {
class WorldPrefs;
}

class AliasModel;
class PluginModel;
class ScriptApi;
class SmushClient;
class TimerModel;
class TriggerModel;
class World;

class WorldPrefs : public QDialog {
  Q_OBJECT

public:
  WorldPrefs(World &world, SmushClient &client, ScriptApi *api,
             QWidget *parent = nullptr);
  ~WorldPrefs();

  constexpr bool isDirty() const { return dirty; }

private:
  void connectModel(QAbstractItemModel *model);
  QWidget *paneForIndex(int row);

private slots:
  void markDirty();
  void on_settings_list_currentRowChanged(int row);

private:
  Ui::WorldPrefs *ui;
  AliasModel *aliases;
  ScriptApi *api;
  SmushClient &client;
  bool dirty = false;
  QWidget *pane = nullptr;
  PluginModel *plugins;
  TimerModel *timers;
  TriggerModel *triggers;
  World &world;
};
