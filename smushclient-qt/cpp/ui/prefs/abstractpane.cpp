#include "abstractpane.h"

AbstractPrefsPane::AbstractPrefsPane(QWidget *parent) : QWidget(parent) {}

// bool
QMetaObject::Connection AbstractPrefsPane::connectWorld(const World *world, QCheckBox *input, const bool value, void (World::*&&setter)(const bool &value))
{
  input->setChecked(value);
  return connect(input, &QCheckBox::checkStateChanged, world, setter);
}

// double
QMetaObject::Connection AbstractPrefsPane::connectWorld(const World *world, QDoubleSpinBox *input, const double value, void (World::*&&setter)(const double &value))
{
  input->setValue(value);
  return connect(input, &QDoubleSpinBox::valueChanged, world, setter);
}

// int
QMetaObject::Connection AbstractPrefsPane::connectWorld(const World *world, QSpinBox *input, const int value, void (World::*&&setter)(const int &value))
{
  input->setValue(value);
  return connect(input, &QSpinBox::valueChanged, world, setter);
}

// qcolor
QMetaObject::Connection AbstractPrefsPane::connectWorld(const World *world, ColorPickerButton *input, const QColor &value, void (World::*&&setter)(const QColor &value))
{
  input->setValue(value);
  return connect(input, &ColorPickerButton::valueChanged, world, setter);
}

// qstring
QMetaObject::Connection AbstractPrefsPane::connectWorld(const World *world, QLineEdit *input, const QString &value, void (World::*&&setter)(const QString &value))
{
  input->setText(value);
  return connect(input, &QLineEdit::textChanged, world, setter);
}
