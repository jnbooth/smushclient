#ifndef WORLDPREFS_H
#define WORLDPREFS_H

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
  explicit WorldPrefs(World *world, QWidget *parent = nullptr);
  ~WorldPrefs();

private slots:
  void on_settings_tree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
  Ui::WorldPrefs *ui;
  QList<QWidget *> panes;
  QWidget *activePane;
  void setupPane(QWidget *pane, const char *key);
};

#endif // WORLDPREFS_H
