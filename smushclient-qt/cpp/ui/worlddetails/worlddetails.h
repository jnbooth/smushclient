#pragma once
#include <QtCore/QList>
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidgetItem>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class WorldPrefs;
}

class ScriptApi;

class WorldPrefs : public QDialog
{
  Q_OBJECT

public:
  explicit WorldPrefs(World &world, SmushClient &client, ScriptApi *api, QWidget *parent = nullptr);
  ~WorldPrefs();

private:
  Ui::WorldPrefs *ui;
  ScriptApi *api;
  SmushClient &client;
  QWidget *pane;
  World &world;

  QWidget *paneForIndex(int row);

private slots:
  void on_settings_list_currentRowChanged(int row);
};
