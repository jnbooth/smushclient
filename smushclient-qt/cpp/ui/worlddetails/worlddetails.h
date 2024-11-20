#pragma once
#include <QtCore/QList>
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidgetItem>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class WorldPrefs;
}

class AliasModel;
class PluginModel;
class ScriptApi;
class TimerModel;
class TriggerModel;

class WorldPrefs : public QDialog
{
  Q_OBJECT

public:
  WorldPrefs(World &world, SmushClient &client, ScriptApi *api, QWidget *parent = nullptr);
  ~WorldPrefs();

  constexpr bool isDirty() const { return dirty; }

private:
  Ui::WorldPrefs *ui;
  AliasModel *aliases;
  ScriptApi *api;
  SmushClient &client;
  bool dirty;
  QWidget *pane;
  PluginModel *plugins;
  TimerModel *timers;
  TriggerModel *triggers;
  World &world;

private:
  void connectModel(QAbstractItemModel *model);
  QWidget *paneForIndex(int row);

private slots:
  void markDirty();
  void on_settings_list_currentRowChanged(int row);
};
