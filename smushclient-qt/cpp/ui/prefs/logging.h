#ifndef PREFSLOGGING_H
#define PREFSLOGGING_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsLogging;
}

class PrefsLogging : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsLogging(QWidget *parent = nullptr);
    ~PrefsLogging();

private:
    Ui::PrefsLogging *ui;
};

#endif // PREFSLOGGING_H
