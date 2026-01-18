#pragma once
#include "./ui/components/colorpickerbutton.h"
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTimeEdit>

#define CONNECT_SETTINGS(field)                                                \
  connectField(                                                                \
    this, &settings, ui->field, settings.get##field(), &Settings::set##field);

#define CONNECT_WORLD(field)                                                   \
  connectField(this, &world, ui->field, world.get##field(), &World::set##field);

template<typename T, typename Enum>
inline QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QComboBox* input,
             const Enum value,
             void (T::*&& setter)(Enum value))
{
  static_assert(std::is_same_v<std::underlying_type_t<Enum>, int>,
                "enum must be represented by int");
  typedef void (T::* && Setter)(const int& value);
  input->setCurrentIndex((int)value);
  return object->connect(input,
                         &QComboBox::currentIndexChanged,
                         target,
                         reinterpret_cast<Setter>(setter));
}

template<typename T>
inline QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QComboBox* input,
             int value,
             void (T::*&& setter)(int value))
{
  input->setCurrentIndex(value);
  return object->connect(
    input, &QComboBox::currentIndexChanged, target, setter);
}

// bool
template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QCheckBox* input,
             const bool value,
             void (T::*&& setter)(bool value))
{
  input->setChecked(value);
  return object->connect(input, &QCheckBox::checkStateChanged, target, setter);
}

// double
template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QDoubleSpinBox* input,
             const double value,
             void (T::*&& setter)(double value))
{
  input->setValue(value);
  return object->connect(input, &QDoubleSpinBox::valueChanged, target, setter);
}

// int
template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QSpinBox* input,
             const int value,
             void (T::*&& setter)(int value))
{
  input->setValue(value);
  return object->connect(input, &QSpinBox::valueChanged, target, setter);
}

// qcolor
template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             ColorPickerButton* input,
             const QColor& value,
             void (T::*&& setter)(QColor value))
{
  input->setValue(value);
  return object->connect(
    input, &ColorPickerButton::valueChanged, target, setter);
}

template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             ColorPickerButton* input,
             const QColor& value,
             void (T::*&& setter)(const QColor& value))
{
  input->setValue(value);
  return object->connect(
    input, &ColorPickerButton::valueChanged, target, setter);
}

// qstring
template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QLineEdit* input,
             const QString& value,
             void (T::*&& setter)(QString value))
{
  input->setText(value);
  return object->connect(input, &QLineEdit::textChanged, target, setter);
}

template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QLineEdit* input,
             const QString& value,
             void (T::*&& setter)(const QString& value))
{
  input->setText(value);
  return object->connect(input, &QLineEdit::textChanged, target, setter);
}

// qtime
template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QTimeEdit* input,
             const QTime& value,
             void (T::*&& setter)(QTime value))
{
  input->setTime(value);
  return object->connect(input, &QTimeEdit::timeChanged, target, setter);
}

template<typename T>
QMetaObject::Connection
connectField(QObject* object,
             const T* target,
             QTimeEdit* input,
             const QTime& value,
             void (T::*&& setter)(const QTime& value))
{
  input->setTime(value);
  return object->connect(input, &QTimeEdit::timeChanged, target, setter);
}
