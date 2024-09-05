#ifndef FIELDCONNECTOR_H
#define FIELDCONNECTOR_H

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QTimeEdit>
#include <QtWidgets/QWidget>
#include "./ui/components/colorpickerbutton.h"

#define CONNECT_WORLD(field) connectField(this, world, ui->field, world->get##field(), &World::set##field);

template <typename T, typename Enum>
inline QMetaObject::Connection connectField(QObject *object, const T *target, QComboBox *input, const Enum value, void (T::*&&setter)(const Enum &value))
{
  static_assert(sizeof(Enum) == sizeof(quint32), "enum must be represented by quint32");
  typedef void (T::* && Setter)(const quint32 &value);
  input->setCurrentIndex((int)value);
  return object->connect(input, &QComboBox::currentIndexChanged, target, reinterpret_cast<Setter>(setter));
}

// bool
template <typename T>
QMetaObject::Connection connectField(QObject *object, const T *target, QCheckBox *input, const bool value, void (T::*&&setter)(const bool &value))
{
  input->setChecked(value);
  return object->connect(input, &QCheckBox::checkStateChanged, target, setter);
}

// double
template <typename T>
QMetaObject::Connection connectField(QObject *object, const T *target, QDoubleSpinBox *input, const double value, void (T::*&&setter)(const double &value))
{
  input->setValue(value);
  return object->connect(input, &QDoubleSpinBox::valueChanged, target, setter);
}

// int
template <typename T>
QMetaObject::Connection connectField(QObject *object, const T *target, QSpinBox *input, const int value, void (T::*&&setter)(const int &value))
{
  input->setValue(value);
  return object->connect(input, &QSpinBox::valueChanged, target, setter);
}

// qcolor
template <typename T>
QMetaObject::Connection connectField(QObject *object, const T *target, ColorPickerButton *input, const QColor &value, void (T::*&&setter)(const QColor &value))
{
  input->setValue(value);
  return object->connect(input, &ColorPickerButton::valueChanged, target, setter);
}

// qstring
template <typename T>
QMetaObject::Connection connectField(QObject *object, const T *target, QLineEdit *input, const QString &value, void (T::*&&setter)(const QString &value))
{
  input->setText(value);
  return object->connect(input, &QLineEdit::textChanged, target, setter);
}

// qtime
template <typename T>
QMetaObject::Connection connectField(QObject *object, const T *target, QTimeEdit *input, const QTime &value, void (T::*&&setter)(const QTime &value))
{
  input->setTime(value);
  return object->connect(input, &QTimeEdit::timeChanged, target, setter);
}

#endif // FIELDCONNECTOR_H
