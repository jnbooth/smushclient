#pragma once
#include "abstractprefstree.h"

namespace Ui {
class PrefsAliases;
} // namespace Ui

class AliasModel;
class World;

class PrefsAliases : public AbstractPrefsTree
{
  Q_OBJECT

public:
  PrefsAliases(const World& world,
               AliasModel* model,
               QWidget* parent = nullptr);
  ~PrefsAliases();

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsAliases* ui;
};
