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
  FieldConnector::connect(                                                     \
    this, &settings, ui->field, settings.get##field(), &Settings::set##field);

#define CONNECT_WORLD(field)                                                   \
  FieldConnector::connect(                                                     \
    this, &world, ui->field, world.get##field(), &World::set##field);

class FieldConnector
{
  template<typename Source, typename Value>
  using Setter = void (Source::*&&)(Value);

public:
  template<typename T, typename Enum>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QComboBox* input,
                                         const Enum value,
                                         Setter<T, Enum> setter)
  {
    static_assert(std::is_same_v<std::underlying_type_t<Enum>, int>,
                  "enum must be represented by int");
    input->setCurrentIndex(static_cast<int>(value));
    return object->connect(input,
                           &QComboBox::currentIndexChanged,
                           target,
                           reinterpret_cast<Setter<T, int>>(setter));
  }

  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QComboBox* input,
                                         int value,
                                         Setter<T, int> setter)
  {
    input->setCurrentIndex(value);
    return object->connect(
      input, &QComboBox::currentIndexChanged, target, setter);
  }

  // bool
  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QCheckBox* input,
                                         const bool value,
                                         Setter<T, bool> setter)
  {
    input->setChecked(value);
    return object->connect(
      input, &QCheckBox::checkStateChanged, target, setter);
  }

  // double
  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QDoubleSpinBox* input,
                                         const double value,
                                         Setter<T, double> setter)
  {
    input->setValue(value);
    return object->connect(
      input, &QDoubleSpinBox::valueChanged, target, setter);
  }

  // int
  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QSpinBox* input,
                                         const int value,
                                         Setter<T, int> setter)
  {
    input->setValue(value);
    return object->connect(input, &QSpinBox::valueChanged, target, setter);
  }

  // qcolor
  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         ColorPickerButton* input,
                                         const QColor& value,
                                         Setter<T, QColor> setter)
  {
    input->setValue(value);
    return object->connect(
      input, &ColorPickerButton::valueChanged, target, setter);
  }

  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         ColorPickerButton* input,
                                         const QColor& value,
                                         Setter<T, const QColor&> setter)
  {
    input->setValue(value);
    return object->connect(
      input, &ColorPickerButton::valueChanged, target, setter);
  }

  // qstring
  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QLineEdit* input,
                                         const QString& value,
                                         Setter<T, QString> setter)
  {
    input->setText(value);
    return object->connect(input, &QLineEdit::textChanged, target, setter);
  }

  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QLineEdit* input,
                                         const QString& value,
                                         Setter<T, const QString&> setter)
  {
    input->setText(value);
    return object->connect(input, &QLineEdit::textChanged, target, setter);
  }

  // qtime
  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QTimeEdit* input,
                                         const QTime& value,
                                         Setter<T, QTime> setter)
  {
    input->setTime(value);
    return object->connect(input, &QTimeEdit::timeChanged, target, setter);
  }

  template<typename T>
  static QMetaObject::Connection connect(QObject* object,
                                         const T* target,
                                         QTimeEdit* input,
                                         const QTime& value,
                                         Setter<T, const QTime&> setter)
  {
    input->setTime(value);
    return object->connect(input, &QTimeEdit::timeChanged, target, setter);
  }
};
