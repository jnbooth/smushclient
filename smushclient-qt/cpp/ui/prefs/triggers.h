#pragma once
#include "abstractprefstree.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsTriggers;
}

class PrefsTriggers : public AbstractPrefsTree
{
  Q_OBJECT

public:
  explicit PrefsTriggers(World &world, QWidget *parent = nullptr);
  ~PrefsTriggers();

protected:
  void addItem() override;
  void editItem(size_t index) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;
  QTreeWidget *tree() const override;

private:
  Ui::PrefsTriggers *ui;
  World &world;

  void buildTree();
};
