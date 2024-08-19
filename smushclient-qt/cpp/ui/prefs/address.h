#ifndef PREFSADDRESS_H
#define PREFSADDRESS_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsAddress;
}

class PrefsAddress : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsAddress(QWidget *parent = nullptr);
    ~PrefsAddress();

private:
    Ui::PrefsAddress *ui;
};

#endif // PREFSADDRESS_H
