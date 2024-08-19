#ifndef PREFSTRIGGERS_H
#define PREFSTRIGGERS_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsTriggers;
}

class PrefsTriggers : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsTriggers(QWidget *parent = nullptr);
    ~PrefsTriggers();

private:
    Ui::PrefsTriggers *ui;
};

#endif // PREFSTRIGGERS_H
