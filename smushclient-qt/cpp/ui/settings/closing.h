#pragma once
#include <QtWidgets/QWidget>

namespace Ui {
class SettingsClosing;
}

class Settings;

class SettingsClosing : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsClosing(Settings &settings, QWidget *parent = nullptr);
    ~SettingsClosing();

private:
    Ui::SettingsClosing *ui;
};
