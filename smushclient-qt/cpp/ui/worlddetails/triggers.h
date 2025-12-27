#pragma once
#include "abstractprefstree.h"

namespace Ui {
class PrefsTriggers;
}

class TriggerModel;
class World;

class PrefsTriggers : public AbstractPrefsTree
{
  Q_OBJECT

public:
  explicit PrefsTriggers(const World& world,
                         TriggerModel* model,
                         QWidget* parent = nullptr);
  ~PrefsTriggers();

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsTriggers* ui;
};
