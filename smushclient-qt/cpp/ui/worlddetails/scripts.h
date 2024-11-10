#pragma once
#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsScripts;
}

class PrefsScripts : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsScripts(World &world, QWidget *parent = nullptr);
  ~PrefsScripts();

private:
  Ui::PrefsScripts *ui;
  World &world;

private:
  QString defaultScriptPath() const;

private slots:
  void on_WorldScript_browse_clicked();
  void on_WorldScript_create_clicked();
  void on_WorldScript_edit_clicked();
  void on_WorldScript_textChanged(const QString &value);
};
