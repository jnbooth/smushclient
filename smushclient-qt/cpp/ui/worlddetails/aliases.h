#pragma once
#include "abstractprefstree.h"
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsAliases;
}

class AliasModel;

class PrefsAliases : public AbstractPrefsTree
{
  Q_OBJECT

public:
  PrefsAliases(const World &world, AliasModel *model, QWidget *parent = nullptr);
  ~PrefsAliases();

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsAliases *ui;
};
