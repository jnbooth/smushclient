#ifndef PREFSALIASES_H
#define PREFSALIASES_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"
#include "abstractprefstree.h"

namespace Ui
{
  class PrefsAliases;
}

class PrefsAliases : public AbstractPrefsTree
{
  Q_OBJECT

public:
  explicit PrefsAliases(World &world, QWidget *parent = nullptr);
  ~PrefsAliases();

protected:
  void addItem() override;
  void editItem(size_t index) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;
  QTreeWidget *tree() const override;

private:
  Ui::PrefsAliases *ui;
  World &world;
  TreeBuilder builder;

  void buildTree();
};

#endif // PREFSALIASES_H
