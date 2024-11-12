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
  explicit PrefsTriggers(const World &world, SmushClient &client, QWidget *parent = nullptr);
  ~PrefsTriggers();

protected:
  bool addItem() override;
  void buildTree(ModelBuilder &builder) override;
  bool editItem(size_t index) override;
  QString exportXml() const override;
  void importXml(const QString &text) override;
  void removeItem(size_t index) override;
  void setItemButtonsEnabled(bool enabled) override;

private:
  Ui::PrefsTriggers *ui;
  SmushClient &client;
};
