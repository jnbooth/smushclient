#ifndef PREFSCONNECTING_H
#define PREFSCONNECTING_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsConnecting;
}

class PrefsConnecting : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsConnecting(QWidget *parent = nullptr);
    ~PrefsConnecting();

private:
    Ui::PrefsConnecting *ui;
};

#endif // PREFSCONNECTING_H
