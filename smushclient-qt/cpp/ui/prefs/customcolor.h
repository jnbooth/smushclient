#ifndef PREFSCUSTOMCOLOR_H
#define PREFSCUSTOMCOLOR_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsCustomColor;
}

class PrefsCustomColor : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsCustomColor(QWidget *parent = nullptr);
    ~PrefsCustomColor();

private:
    Ui::PrefsCustomColor *ui;
};

#endif // PREFSCUSTOMCOLOR_H
