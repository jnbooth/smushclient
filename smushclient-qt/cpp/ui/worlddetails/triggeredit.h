#pragma once
#include "smushclient_qt/src/ffi/audio.cxxqt.h"
#include <QtWidgets/QDialog>

namespace Ui {
class TriggerEdit;
}

class Trigger;

class TriggerEdit : public QDialog {
  Q_OBJECT

public:
  explicit TriggerEdit(Trigger &trigger, QWidget *parent = nullptr);
  ~TriggerEdit();

  bool groupChanged() const;

private slots:
  void on_Label_textChanged(const QString &text);
  void on_UserSendTo_currentIndexChanged(int index);
  void on_Sound_browse_clicked();
  void on_Sound_test_clicked();
  void on_Sound_textChanged(const QString &text);
  void on_Text_textChanged();

private:
  Ui::TriggerEdit *ui;
  RustFilePlayback audio{};
  QString originalGroup;
  Trigger &trigger;
};
