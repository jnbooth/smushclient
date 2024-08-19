#ifndef PREFSTIMERS_H
#define PREFSTIMERS_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsTimers;
}

class PrefsTimers : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsTimers(QWidget *parent = nullptr);
    ~PrefsTimers();

private:
    Ui::PrefsTimers *ui;
};

#endif // PREFSTIMERS_H
