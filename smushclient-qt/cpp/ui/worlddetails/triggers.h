#pragma once
#include "abstractprefstree.h"

namespace Ui {
class PrefsTriggers;
} // namespace Ui

class TriggerModel;
class World;

class PrefsTriggers : public AbstractPrefsTree
{
  Q_OBJECT

public:
  explicit PrefsTriggers(const World& world,
                         TriggerModel& model,
                         QWidget* parent = nullptr);
  ~PrefsTriggers() override;

protected:
  void enableSingleButtons(bool enabled) override;
  void enableMultiButtons(bool enabled) override;

private:
  Ui::PrefsTriggers* ui;
};
