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
  explicit PrefsTimers(const World &world, SmushClient &client, Timekeeper *timekeeper, QWidget *parent = nullptr);
  ~PrefsTimers();

protected:
  bool addItem() override;
  void buildTree(TreeBuilder &builder) override;
  bool editItem(size_t index) override;
  QString exportXml() const override;
  void importXml(const QString &text) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;
  QTreeWidget *tree() const override;

private:
  Ui::PrefsTimers *ui;
  SmushClient &client;
  Timekeeper *timekeeper;
};
