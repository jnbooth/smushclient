#ifndef WORLDTAB_H
#define WORLDTAB_H

#include <QtWidgets/QSplitter>

namespace Ui {
class WorldTab;
}

class WorldTab : public QSplitter
{
    Q_OBJECT

public:
    explicit WorldTab(QWidget *parent = nullptr);
    ~WorldTab();

private:
    Ui::WorldTab *ui;
};

#endif // WORLDTAB_H
