#ifndef PREFSPROXY_H
#define PREFSPROXY_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>

namespace Ui {
class PrefsProxy;
}

class PrefsProxy : public QDialog
{
    Q_OBJECT

public:
    explicit PrefsProxy(QWidget *parent = nullptr);
    ~PrefsProxy();

private:
    Ui::PrefsProxy *ui;
};

#endif // PREFSPROXY_H
