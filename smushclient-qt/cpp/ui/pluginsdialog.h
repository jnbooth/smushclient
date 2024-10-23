#ifndef PLUGINSDIALOG_H
#define PLUGINSDIALOG_H

#include <QDialog>

namespace Ui
{
  class PluginsDialog;
}

class SmushClient;

class PluginsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PluginsDialog(SmushClient &client, QWidget *parent = nullptr);
  ~PluginsDialog();

signals:
  void reinstallClicked();

private:
  Ui::PluginsDialog *ui;
  SmushClient &client;
  bool changedPlugins;

  void buildTable();

private slots:
  void on_button_add_clicked();
  void on_button_close_clicked();
  void on_button_reinstall_clicked();
  void on_button_remove_clicked();
};

#endif // PLUGINSDIALOG_H
