#pragma once
#include <QtWidgets/QDialog>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QWidget>
#include <QtCore/QList>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class WorldPrefs;
}

class WorldPrefs : public QDialog
{
  Q_OBJECT

public:
  explicit WorldPrefs(World &world, QWidget *parent = nullptr);
  ~WorldPrefs();

private:
  Ui::WorldPrefs *ui;
  QList<QWidget *> panes;
  qsizetype activePane;

  void setupPane(QWidget *pane, const char *key);

private slots:
  void on_settings_tree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
};
