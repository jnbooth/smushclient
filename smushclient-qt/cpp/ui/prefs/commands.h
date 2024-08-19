#ifndef PREFSCOMMANDS_H
#define PREFSCOMMANDS_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsCommands;
}

class PrefsCommands : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsCommands(QWidget *parent = nullptr);
    ~PrefsCommands();

private:
    Ui::PrefsCommands *ui;
};

#endif // PREFSCOMMANDS_H
