#ifndef PLUGINPOPUP_H
#define PLUGINPOPUP_H

#include <QDialog>

class SmushClient;

namespace Ui {
class PluginPopup;
}

class PluginPopup : public QDialog
{
    Q_OBJECT

public:
    PluginPopup(const SmushClient &client, const QString &id, QWidget *parent = nullptr);
    ~PluginPopup();

private:
    Ui::PluginPopup *ui;
};

#endif // PLUGINPOPUP_H
