#pragma once
#include <QtWidgets/QDialog>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class TriggerEdit;
}

class TriggerEdit : public QDialog
{
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
  QAudioOutput audio;
  QMediaPlayer player;
  QString originalGroup;
  Trigger &trigger;
};
