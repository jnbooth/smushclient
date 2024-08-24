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
  template <typename Enum>
  inline QMetaObject::Connection connectWorld(const World *world, QComboBox *input, const Enum value, void (World::*&&setter)(const Enum &value))
  {
    static_assert(sizeof(Enum) == sizeof(quint32), "enum must be represented by quint32");
    typedef void (World::* && Setter)(const quint32 &value);
    input->setCurrentIndex((int)value);
    return connect(input, &QComboBox::currentIndexChanged, world, reinterpret_cast<Setter>(setter));
  }
  // bool
  QMetaObject::Connection connectWorld(const World *world, QCheckBox *input, const bool value, void (World::*&&setter)(const bool &value));
  // double
  QMetaObject::Connection connectWorld(const World *world, QDoubleSpinBox *input, const double value, void (World::*&&setter)(const double &value));
  // int
  QMetaObject::Connection connectWorld(const World *world, QSpinBox *input, const int value, void (World::*&&setter)(const int &value));
  // qcolor
  QMetaObject::Connection connectWorld(const World *world, ColorPickerButton *input, const QColor &value, void (World::*&&setter)(const QColor &value));
  // qstring
  QMetaObject::Connection connectWorld(const World *world, QLineEdit *input, const QString &value, void (World::*&&setter)(const QString &value));
};

#endif // PREFSABSTRACTPANE_H
