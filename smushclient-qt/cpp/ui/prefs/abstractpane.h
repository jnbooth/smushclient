#ifndef PREFSABSTRACTPANE_H
#define PREFSABSTRACTPANE_H

#include "cxx-qt-gen/ffi.cxxqt.h"
#include "../components/colorpickerbutton.h"
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QWidget>

#define CONNECT_WORLD(field) connectWorld(world, ui->field, world->get##field(), &World::set##field);

class AbstractPrefsPane : public QWidget
{
  Q_OBJECT

public:
  explicit AbstractPrefsPane(QWidget *parent = nullptr);

protected:
  template <typename T, typename Enum>
  inline QMetaObject::Connection connectWorld(const T *world, QComboBox *input, const Enum value, void (T::*&&setter)(const Enum &value))
  {
    static_assert(sizeof(Enum) == sizeof(quint32), "enum must be represented by quint32");
    typedef void (T::* && Setter)(const quint32 &value);
    input->setCurrentIndex((int)value);
    return connect(input, &QComboBox::currentIndexChanged, world, reinterpret_cast<Setter>(setter));
  }

  // bool
  template <typename T>
  QMetaObject::Connection connectWorld(const T *world, QCheckBox *input, const bool value, void (T::*&&setter)(const bool &value))
  {
    input->setChecked(value);
    return connect(input, &QCheckBox::checkStateChanged, world, setter);
  }

  // double
  template <typename T>
  QMetaObject::Connection connectWorld(const T *world, QDoubleSpinBox *input, const double value, void (T::*&&setter)(const double &value))
  {
    input->setValue(value);
    return connect(input, &QDoubleSpinBox::valueChanged, world, setter);
  }

  // int
  template <typename T>
  QMetaObject::Connection connectWorld(const T *world, QSpinBox *input, const int value, void (T::*&&setter)(const int &value))
  {
    input->setValue(value);
    return connect(input, &QSpinBox::valueChanged, world, setter);
  }

  // qcolor
  template <typename T>
  QMetaObject::Connection connectWorld(const T *world, ColorPickerButton *input, const QColor &value, void (T::*&&setter)(const QColor &value))
  {
    input->setValue(value);
    return connect(input, &ColorPickerButton::valueChanged, world, setter);
  }

  // qstring
  template <typename T>
  QMetaObject::Connection connectWorld(const T *world, QLineEdit *input, const QString &value, void (T::*&&setter)(const QString &value))
  {
    input->setText(value);
    return connect(input, &QLineEdit::textChanged, world, setter);
  }
};

#endif // PREFSABSTRACTPANE_H
