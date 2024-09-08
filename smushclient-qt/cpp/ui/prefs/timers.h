#pragma once
#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"
#include "abstractprefstree.h"

namespace Ui
{
  class PrefsTimers;
}

class PrefsTimers : public AbstractPrefsTree
{
  Q_OBJECT

public:
  explicit PrefsTimers(World &world, QWidget *parent = nullptr);
  ~PrefsTimers();

protected:
  void addItem() override;
  void editItem(size_t index) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;
  QTreeWidget *tree() const override;

private:
  Ui::PrefsTimers *ui;
  World &world;
  TreeBuilder builder;

  void buildTree();
};
