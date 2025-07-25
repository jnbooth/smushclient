#pragma once
#include "abstractscriptdialog.h"
#include <QtWidgets/QListWidgetItem>

namespace Ui {
class ListBox;
}

class ListBox : public AbstractScriptDialog {
  Q_OBJECT

public:
  explicit ListBox(const QString &title, const QString &message,
                   QWidget *parent = nullptr);
  ~ListBox();

  void addItems(const QStringList &items);
  void setMode(QListWidget::SelectionMode mode);
  QString text() const;

  void addItem(const QString &text, const QVariant &value,
               bool active = false) override;
  void sortItems() override;
  QVariant value() const override;

private slots:
  void on_items_itemDoubleClicked(QListWidgetItem *item);

private:
  Ui::ListBox *ui;
};
