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
  explicit PrefsAliases(const World &world, SmushClient &client, QWidget *parent = nullptr);
  ~PrefsAliases();

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsAliases *ui;
  AliasModel *model;
};
