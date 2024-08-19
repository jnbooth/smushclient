#ifndef PREFSMXP_H
#define PREFSMXP_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsMxp;
}

class PrefsMxp : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsMxp(QWidget *parent = nullptr);
    ~PrefsMxp();

private:
    Ui::PrefsMxp *ui;
};

#endif // PREFSMXP_H
