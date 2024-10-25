#pragma once
#include "abstractprefstree.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

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
  bool addItem() override;
  void buildTree(TreeBuilder &builder) override;
  bool editItem(size_t index) override;
  QString exportXml() const override;
  QString importXml(const QString &text) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;
  QTreeWidget *tree() const override;

private:
  Ui::PrefsAliases *ui;
  World &world;

};
