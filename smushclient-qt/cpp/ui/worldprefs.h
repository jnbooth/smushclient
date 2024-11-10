#pragma once
#include <QtCore/QList>
#include <QtWidgets/QDialog>
#include <QtWidgets/QListWidgetItem>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class WorldPrefs;
}

class WorldPrefs : public QDialog
{
  Q_OBJECT

public:
  explicit WorldPrefs(World &world, SmushClient &client, Timekeeper *timekeeper, QWidget *parent = nullptr);
  ~WorldPrefs();

private:
  Ui::WorldPrefs *ui;
  QList<QWidget *> panes;
  qsizetype activePane;

  void setupPane(QWidget *pane, const char *key);

private slots:
  void on_settings_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
};
