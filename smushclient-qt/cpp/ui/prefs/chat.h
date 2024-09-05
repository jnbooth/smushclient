#ifndef PREFSCHAT_H
#define PREFSCHAT_H

#include <QtWidgets/QWidget>
#include "cxx-qt-gen/ffi.cxxqt.h"

namespace Ui
{
  class PrefsChat;
}

class PrefsChat : public QWidget
{
  Q_OBJECT

public:
  explicit PrefsChat(const World *world, QWidget *parent = nullptr);
  ~PrefsChat();

private:
  Ui::PrefsChat *ui;
};

#endif // PREFSCHAT_H
