#pragma once
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QButtonGroup>

template <typename T>
class EnumButtonGroup
{
public:
  template <typename Parent>
  EnumButtonGroup(Parent *parent, T value, void (Parent::*&&slot)(T value))
      : group(new QButtonGroup(parent)),
        currentValue(value)
  {
    static_assert(std::is_same_v<std::underlying_type_t<T>, int>, "enum must be represented by int");
    typedef void (Parent::* && IntSlot)(int value);
    group->setExclusive(true);
    parent->connect(group, &QButtonGroup::idClicked, parent, reinterpret_cast<IntSlot>(slot));
  }

  const EnumButtonGroup<T> &addButton(QAbstractButton *button, T id) const
  {
    if (id == currentValue)
      button->setChecked(true);
    group->addButton(button, (int)id);
    return *this;
  }

private:
  QButtonGroup *group;
  T currentValue;
};
