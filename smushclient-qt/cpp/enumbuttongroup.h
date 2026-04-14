#pragma once
#include "casting.h"
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QButtonGroup>

template<IntEnum T>
class EnumButtonGroup
{
  template<typename Source, typename Value>
  using Setter = void (Source::*&&)(Value);

public:
  template<typename Source>
  EnumButtonGroup(QObject* parent,
                  T value,
                  Source* source,
                  Setter<Source, T> setter)
    : group(new QButtonGroup(parent))
    , currentValue(value)
  {
    group->setExclusive(true);
    parent->connect(
      group, &QButtonGroup::idClicked, source, enum_slot_cast(setter));
  }

  const EnumButtonGroup<T>& addButton(QAbstractButton* button, T id) const
  {
    if (id == currentValue) {
      button->setChecked(true);
    }
    group->addButton(button, static_cast<int>(id));
    return *this;
  }

private:
  QButtonGroup* group;
  T currentValue;
};
