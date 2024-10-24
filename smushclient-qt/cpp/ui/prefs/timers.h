#pragma once
#include "abstractprefstree.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

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
  QString exportXml() const override;
  QString importXml(const QString &text) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;
  QTreeWidget *tree() const override;

private:
  Ui::PrefsTimers *ui;
  World &world;

  void buildTree();
};
