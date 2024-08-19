#ifndef PREFSALIASES_H
#define PREFSALIASES_H

#include <QtWidgets/QWidget>

namespace Ui {
class PrefsAliases;
}

class PrefsAliases : public QWidget
{
    Q_OBJECT

public:
    explicit PrefsAliases(QWidget *parent = nullptr);
    ~PrefsAliases();

private:
    Ui::PrefsAliases *ui;
};

#endif // PREFSALIASES_H
