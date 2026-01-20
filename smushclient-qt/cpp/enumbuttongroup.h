#pragma once
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QButtonGroup>

template<typename T>
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
    static_assert(std::is_same_v<std::underlying_type_t<T>, int>,
                  "enum must be represented by int");
    group->setExclusive(true);
    parent->connect(group,
                    &QButtonGroup::idClicked,
                    source,
                    reinterpret_cast<Setter<Source, int>>(setter));
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
