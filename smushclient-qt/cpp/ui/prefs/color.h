#ifndef PREFSCOLOR_H
#define PREFSCOLOR_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsColor;
}

class PrefsColor : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsColor(QWidget *parent = nullptr);
    ~PrefsColor();

private:
    Ui::PrefsColor *ui;
};

#endif // PREFSCOLOR_H
