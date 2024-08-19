#ifndef PREFSOUTPUT_H
#define PREFSOUTPUT_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsOutput;
}

class PrefsOutput : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsOutput(QWidget *parent = nullptr);
    ~PrefsOutput();

private:
    Ui::PrefsOutput *ui;
};

#endif // PREFSOUTPUT_H
