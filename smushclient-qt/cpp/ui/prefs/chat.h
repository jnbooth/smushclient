#ifndef PREFSCHAT_H
#define PREFSCHAT_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsChat;
}

class PrefsChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsChat(QWidget *parent = nullptr);
    ~PrefsChat();

private:
    Ui::PrefsChat *ui;
};

#endif // PREFSCHAT_H
